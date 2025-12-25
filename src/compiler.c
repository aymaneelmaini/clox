#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "object.h"
#include "scanner.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct
{
    Token previous;
    Token current;
    bool  had_error;
    bool  panic_mode;
} Parser;

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARAISON,
    PREC_TERM,  // + -
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,  // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool can_assign);

typedef struct
{
    ParseFn    prefix;
    ParseFn    infix;
    Precedence precedence;
} ParseRule;

typedef struct
{
    Token name;
    int   depth;
    bool  is_immutable;
} Local;

typedef struct
{
    Local locals[UINT8_COUNT];
    int   local_count;
    int   scope_depth;
} Compiler;

static bool immutable_globals[UINT8_MAX];

Parser      parser;
Compiler*   current;
Chunk*      compiling_chunk;

static Chunk* current_chunk()
{
    return compiling_chunk;
}

static inline void error_at(Token* token, const char* message)
{
    if (parser.panic_mode)
        return;

    parser.panic_mode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR)
    {
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}

static void error(const char* message)
{
    error_at(&parser.previous, message);
}

static void error_at_current(const char* message)
{
    error_at(&parser.current, message);
}

static void advance()
{
    parser.previous = parser.current;
    for (;;)
    {
        parser.current = scan_token();
        if (parser.current.type != TOKEN_ERROR)
            break;
        error_at_current(parser.current.start);
    }
}

static void consume(TokenType type, char* message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }
    error_at_current(message);
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}

static bool match(TokenType type)
{
    if (!check(type))
        return false;
    advance();
    return true;
}

static void emit_byte(u8 byte)
{
    write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(u8 byte_1, u8 byte_2)
{
    emit_byte(byte_1);
    emit_byte(byte_2);
}

static void emit_return()
{
    emit_byte(OP_RETURN);
}

static u8 make_constant(Value value)
{
    int constant = add_constant(current_chunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk");
        return 0;
    }
    return (u8)constant;
}

static void emit_constant(Value value)
{
    emit_bytes(OP_CONSTANT, make_constant(value));
}

static void init_compiler(Compiler* compiler)
{
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    current = compiler;
}

static void end_compiler()
{
    emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error)
        disassemble_chunk(current_chunk(), "code");
#endif
}

static void begin_scope()
{
    current->scope_depth++;
}

static void end_scope()
{
    current->scope_depth--;

    while (current->local_count > 0 &&
           current->locals[current->local_count - 1].depth >
               current->scope_depth)
    {
        emit_byte(OP_POP);
        current->local_count--;
    }
}

static void       expression();
static void       statement();
static void       declaration();
static ParseRule* get_rule(TokenType type);
static void       parse_precedence(Precedence precedence);

static u8 identifier_constant(Token* name)
{
    return make_constant(OBJ_VAL(copy_string(name->start, name->length)));
}

static bool identifiers_equal(Token* t1, Token* t2)
{
    if (t1->length != t2->length)
        return false;

    return memcmp(t1->start, t2->start, t1->length) == 0;
}

static int resolve_local(Compiler* compiler, Token* name)
{
    for (int i = compiler->local_count - 1; i >= 0; i--)
    {
        Local* local = &compiler->locals[i];
        if (identifiers_equal(name, &local->name))
        {
            if (local->depth == -1)
                error("Can't read local variable in its initializer");

            return i;
        }
    }
    return -1;
}

static void add_local(Token name, bool is_immutable)
{
    if (current->local_count == UINT8_COUNT)
    {
        error("Too many variables in function");
        return;
    }
    Local* local = &current->locals[current->local_count++];
    local->name = name;
    local->is_immutable = is_immutable;
    local->depth = -1;
}

static void declare_variable(bool is_immutable)
{
    if (current->scope_depth == 0)
        return;

    Token* name = &parser.previous;

    for (int i = current->local_count - 1; i >= 0; i--)
    {
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scope_depth)
            break;

        if (identifiers_equal(name, &local->name))
            error("Already variable with this name in this scope");
    }
    add_local(*name, is_immutable);
}

static u8 parse_variable(bool is_immutable, char* error_message)
{
    consume(TOKEN_IDENTIFIER, error_message);

    declare_variable(is_immutable);
    if (current->scope_depth > 0)
        return 0;

    return identifier_constant(&parser.previous);
}

static void mark_initialized()
{
    current->locals[current->local_count - 1].depth = current->scope_depth;
}

static void define_variable(u8 global, bool is_immutable)
{
    if (current->scope_depth > 0)
    {
        mark_initialized();
        return;
    }
    immutable_globals[global] = is_immutable;
    emit_bytes(OP_DEFINE_GLOBAL, global);
}

static void binary(bool can_assign)
{
    TokenType operator_type = parser.previous.type;

    ParseRule* rule = get_rule(operator_type);
    parse_precedence((Precedence)(rule->precedence + 1));

    switch (operator_type)
    {
    case TOKEN_PLUS:
        emit_byte(OP_ADD);
        break;
    case TOKEN_BANG_EQUAL:
        emit_bytes(OP_EQUAL, OP_NOT);
        break;
    case TOKEN_EQUAL_EQUAL:
        emit_byte(OP_EQUAL);
        break;
    case TOKEN_GREATER:
        emit_byte(OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        emit_bytes(OP_LESS, OP_NOT);
        break;
    case TOKEN_LESS:
        emit_byte(OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        emit_bytes(OP_GREATER, OP_NOT);
        break;
    case TOKEN_MINUS:
        emit_byte(OP_SUBSTRACT);
        break;
    case TOKEN_STAR:
        emit_byte(OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        emit_byte(OP_DIVIDE);
        break;
    default:
        return;
    }
}

static void literal(bool can_assign)
{
    switch (parser.previous.type)
    {
    case TOKEN_NIL:
        emit_byte(OP_NIL);
        break;
    case TOKEN_TRUE:
        emit_byte(OP_TRUE);
        break;
    case TOKEN_FALSE:
        emit_byte(OP_FALSE);
        break;
    default:
        return;
    }
}

static void grouping(bool can_assign)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void number(bool can_assign)
{
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

static void string(bool can_assign)
{
    emit_constant(OBJ_VAL(
        copy_string(parser.previous.start + 1, parser.previous.length - 2)));
}

static void named_variable(Token name, bool can_assign)
{
    u8  get_op, set_op;
    int arg = resolve_local(current, &name);

    if (arg != -1)
    {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    }
    else
    {
        arg = identifier_constant(&name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (can_assign && match(TOKEN_EQUAL))
    {
        if (get_op == OP_GET_LOCAL)
        {
            if (current->locals[arg].is_immutable == true)
            {
                error("Cannot reassign immutable variables");
                return;
            }
        }
        else
        {
            if (immutable_globals[arg])
            {
                error("Cannot reassign immutable variables");
            }
        }
        expression();
        emit_bytes(set_op, (u8)arg);
    }
    else
    {
        emit_bytes(get_op, (u8)arg);
    }
}

static void variable(bool can_assign)
{
    named_variable(parser.previous, can_assign);
}

static void unary(bool can_assign)
{
    TokenType operator_type = parser.previous.type;

    parse_precedence(PREC_UNARY);

    switch (operator_type)
    {
    case TOKEN_BANG:
        emit_byte(OP_NOT);
        break;
    case TOKEN_MINUS:
        emit_byte(OP_NEGATE);
        break;
    default:
        return;
    }
}
// clang-format off
ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]     =  { grouping,  NULL,    PREC_NONE        },
    [TOKEN_RIGHT_PAREN]    =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_LEFT_BRACE]     =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_RIGHT_BRACE]    =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_COMMA]          =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_DOT]            =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_MINUS]          =  { unary,     binary,  PREC_TERM        },
    [TOKEN_PLUS]           =  { NULL,      binary,  PREC_TERM        },
    [TOKEN_SEMICOLON]      =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_SLASH]          =  { NULL,      binary,  PREC_FACTOR      },
    [TOKEN_STAR]           =  { NULL,      binary,  PREC_FACTOR      },
    [TOKEN_BANG]           =  { unary,     NULL,    PREC_NONE        },
    [TOKEN_BANG_EQUAL]     =  { NULL,      binary,  PREC_EQUALITY    },
    [TOKEN_EQUAL]          =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_EQUAL_EQUAL]    =  { NULL,      binary,  PREC_EQUALITY    },
    [TOKEN_GREATER]        =  { NULL,      binary,  PREC_COMPARAISON },
    [TOKEN_GREATER_EQUAL]  =  { NULL,      binary,  PREC_COMPARAISON },
    [TOKEN_LESS]           =  { NULL,      binary,  PREC_COMPARAISON },
    [TOKEN_LESS_EQUAL]     =  { NULL,      binary,  PREC_COMPARAISON },
    [TOKEN_STRING]         =  { string,    NULL,    PREC_NONE        },
    [TOKEN_NUMBER]         =  { number,    NULL,    PREC_NONE        },
    [TOKEN_IDENTIFIER]     =  { variable,  NULL,    PREC_NONE        },
    [TOKEN_AND]            =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_CLASS]          =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_ELSE]           =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_FALSE]          =  { literal,   NULL,    PREC_NONE        },
    [TOKEN_FOR]            =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_FUN]            =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_IF]             =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_NIL]            =  { literal,   NULL,    PREC_NONE        },
    [TOKEN_OR]             =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_PRINT]          =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_RETURN]         =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_SUPER]          =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_THIS]           =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_TRUE]           =  { literal,   NULL,    PREC_NONE        },
    [TOKEN_VAR]            =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_WHILE]          =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_ERROR]          =  { NULL,      NULL,    PREC_NONE        },
    [TOKEN_EOF]            =  { NULL,      NULL,    PREC_NONE        },
};
// clang-format on

static void parse_precedence(Precedence precedence)
{
    advance();
    ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL)
    {
        error("Expect expression");
        return;
    }
    bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefix_rule(can_assign);

    while (precedence <= get_rule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule(can_assign);
    }

    if (can_assign && match(TOKEN_EQUAL))
    {
        error("Invalid assignment target");
    }
}

static ParseRule* get_rule(TokenType type)
{
    return &rules[type];
}

static void expression()
{
    parse_precedence(PREC_ASSIGNMENT);
}

// ---------------------- statements --------------------------

static void block()
{
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block");
}

static void var_declaration()
{
    bool is_immutable = parser.previous.type == TOKEN_VAL;
    u8   global = parse_variable(is_immutable, "Expect variable name");

    if (is_immutable && !check(TOKEN_EQUAL))
    {
        error("Can't declare immutable variable without initializer");
        return;
    }
    if (match(TOKEN_EQUAL))
        expression();
    else
        emit_byte(OP_NIL);

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");
    define_variable(global, is_immutable);
}

static void expression_statement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emit_byte(OP_POP);
}

static void print_statement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' at the end of print statement.");
    emit_byte(OP_PRINT);
}

static void synchronize()
{
    parser.panic_mode = false;

    while (parser.current.type != TOKEN_EOF)
    {
        if (parser.previous.type == TOKEN_SEMICOLON)
            return;

        switch (parser.current.type)
        {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
            return;

        default:
            // do nothings
            ;
        }
        advance();
    }
}

static void declaration()
{
    if (match(TOKEN_VAR) || match(TOKEN_VAL))
        var_declaration();
    else
        statement();

    if (parser.panic_mode)
        synchronize();
}

static void statement()
{
    if (match(TOKEN_PRINT))
    {
        print_statement();
    }
    else if (match(TOKEN_LEFT_BRACE))
    {
        begin_scope();
        block();
        end_scope();
    }
    else
    {
        expression_statement();
    }
}

static inline void init_parser()
{
    parser.had_error = false;
    parser.panic_mode = false;
}

bool compile(const char* source, Chunk* chunk)
{
    init_scanner(source);
    Compiler compiler;
    init_compiler(&compiler);
    compiling_chunk = chunk;
    init_parser();

    advance();
    while (!match(TOKEN_EOF))
    {
        declaration();
    }

    end_compiler();
    return !parser.had_error;
}

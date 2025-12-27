#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "value.h"

static int simple_instruction(const char* name, int offset);
static int constant_instruction(const char* name, Chunk* chunk, int offset);
static int byte_instruction(const char* name, Chunk* chunk, int offset);
static int jump_instruction(const char* name, int sign, Chunk* chunk,
                            int offset);

void disassemble_chunk(Chunk* chunk, const char* name)
{
    printf("=== %s === \n", name);

    for (int offset = 0; offset < chunk->count;)
    {
        offset = disassemble_instruction(chunk, offset);
    }
}

int disassemble_instruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset);

    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
        printf("   | ");
    else
        printf("%4d ", chunk->lines[offset]);

    u8 instruction = chunk->code[offset];
    switch (instruction)
    {
    case OP_CONSTANT:
        return constant_instruction("OP_CONSTANT", chunk, offset);
    case OP_NIL:
        return simple_instruction("OP_NIL", offset);
    case OP_FALSE:
        return simple_instruction("OP_FALSE", offset);
    case OP_TRUE:
        return simple_instruction("OP_TRUE", offset);
    case OP_POP:
        return simple_instruction("OP_POP", offset);
    case OP_GET_LOCAL:
        return byte_instruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
        return byte_instruction("OP_SET_LOCAL", chunk, offset);
    case OP_GET_GLOBAL:
        return constant_instruction("OP_GET_GLOBAL", chunk, offset);
    case OP_DEFINE_GLOBAL:
        return constant_instruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
        return constant_instruction("OP_SET_GLOBAL", chunk, offset);
    case OP_EQUAL:
        return simple_instruction("OP_EQUAL", offset);
    case OP_GREATER:
        return simple_instruction("OP_GREATER", offset);
    case OP_LESS:
        return simple_instruction("OP_LESS", offset);
    case OP_ADD:
        return simple_instruction("OP_ADD", offset);
    case OP_SUBSTRACT:
        return simple_instruction("OP_SUBSTRACT", offset);
    case OP_MULTIPLY:
        return simple_instruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return simple_instruction("OP_DIVIDE", offset);
    case OP_NOT:
        return simple_instruction("OP_NOT", offset);
    case OP_NEGATE:
        return simple_instruction("OP_NEGATE", offset);
    case OP_PRINT:
        return simple_instruction("OP_PRINT", offset);
    case OP_JUMP:
        return jump_instruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
        return jump_instruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_RETURN:
        return simple_instruction("OP_RETURN", offset);
    default:
        printf("Uknown opcode %d \n", instruction);
        return offset + 1;
    }
}

static int simple_instruction(const char* name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static int byte_instruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

static int jump_instruction(const char* name, int sign, Chunk* chunk,
                            int offset)
{
    u16 jump = (u16)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset];
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

static int constant_instruction(const char* name, Chunk* chunk, int offset)
{
    u8 constantIndex = chunk->code[offset + 1];
    printf("%-16s  %4d ", name, constantIndex);
    print_value(chunk->constants.values[constantIndex]);
    printf("\n");
    return offset + 2;
}

static const char* token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TOKEN_LEFT_PAREN:
        return "LEFT_PAREN";
    case TOKEN_RIGHT_PAREN:
        return "RIGHT_PAREN";
    case TOKEN_LEFT_BRACE:
        return "LEFT_BRACE";
    case TOKEN_RIGHT_BRACE:
        return "RIGHT_BRACE";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_DOT:
        return "DOT";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_SLASH:
        return "SLASH";
    case TOKEN_STAR:
        return "STAR";

    case TOKEN_BANG:
        return "BANG";
    case TOKEN_BANG_EQUAL:
        return "BANG_EQUAL";
    case TOKEN_EQUAL:
        return "EQUAL";
    case TOKEN_EQUAL_EQUAL:
        return "EQUAL_EQUAL";
    case TOKEN_GREATER:
        return "GREATER";
    case TOKEN_GREATER_EQUAL:
        return "GREATER_EQUAL";
    case TOKEN_LESS:
        return "LESS";
    case TOKEN_LESS_EQUAL:
        return "LESS_EQUAL";

    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_NUMBER:
        return "NUMBER";

    case TOKEN_AND:
        return "AND";
    case TOKEN_CLASS:
        return "CLASS";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_FALSE:
        return "FALSE";
    case TOKEN_FOR:
        return "FOR";
    case TOKEN_FUN:
        return "FUN";
    case TOKEN_IF:
        return "IF";
    case TOKEN_NIL:
        return "NIL";
    case TOKEN_OR:
        return "OR";
    case TOKEN_PRINT:
        return "PRINT";
    case TOKEN_RETURN:
        return "RETURN";
    case TOKEN_SUPER:
        return "SUPER";
    case TOKEN_THIS:
        return "THIS";
    case TOKEN_TRUE:
        return "TRUE";
    case TOKEN_VAR:
        return "VAR";
    case TOKEN_VAL:
        return "VAL";
    case TOKEN_WHILE:
        return "WHILE";

    case TOKEN_ERROR:
        return "ERROR";
    case TOKEN_EOF:
        return "EOF";
    }

    return "UNKNOWN";
}

void print_token(Token token)
{
    printf("%-15s line %-4d '", token_type_to_string(token.type), token.line);

    for (int i = 0; i < token.length; i++)
    {
        putchar(token.start[i]);
    }

    printf("'\n");
}

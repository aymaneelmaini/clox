#include "../src/scanner.h"
#include <criterion/criterion.h>
#include <stdio.h>

typedef struct
{
    const char* input;
    TokenType   expected_type;
    int         expected_length;
} TokenTest;

static inline void print_token(Token token)
{
    printf("Token { type: %d, line: %d, length: %d, start: '%.*s' } \n",
           token.type, token.line, token.length, token.length, token.start);
}

Test(scanner, should_skip_spaces_and_return_EOF_token)
{
    char* source = "          ";
    init_scanner(source);

    Token result = scan_token();
    cr_assert(result.type == TOKEN_EOF);
}

Test(scanner, should_tokenize_operators)
{
    char* source = "+ - ( ) { } == !=";
    init_scanner(source);

    Token plus = scan_token();
    cr_assert_eq(plus.type, TOKEN_PLUS);

    Token minus = scan_token();
    cr_assert_eq(minus.type, TOKEN_MINUS);

    Token l_paren = scan_token();
    cr_assert_eq(l_paren.type, TOKEN_LEFT_PAREN);

    Token r_paren = scan_token();
    cr_assert_eq(r_paren.type, TOKEN_RIGHT_PAREN);

    Token l_brace = scan_token();
    cr_assert_eq(l_brace.type, TOKEN_LEFT_BRACE);

    Token r_brace = scan_token();
    cr_assert_eq(r_brace.type, TOKEN_RIGHT_BRACE);

    Token equal_equal = scan_token();
    cr_assert_eq(equal_equal.type, TOKEN_EQUAL_EQUAL);

    Token not_equal = scan_token();
    cr_assert_eq(not_equal.type, TOKEN_BANG_EQUAL);

    Token eof = scan_token();
    cr_assert_eq(eof.type, TOKEN_EOF);
}

Test(scanner, should_tokenize_strings)
{
    char* source = "\"hello world\", \"hello clox\"";
    init_scanner(source);

    Token str_1 = scan_token();
    cr_assert_eq(str_1.type, TOKEN_STRING);
    cr_assert_eq(str_1.length, strlen("\"hello world\""));

    Token comma = scan_token();
    cr_assert_eq(comma.type, TOKEN_COMMA);
    cr_assert_eq(comma.length, 1);

    Token str_2 = scan_token();
    cr_assert_eq(str_2.type, TOKEN_STRING);
    cr_assert_eq(str_2.length, strlen("\"hello clox\""));

    cr_assert_eq(str_2.line, 1);

    Token eof = scan_token();
    cr_assert_eq(eof.type, TOKEN_EOF);
}

Test(scanner, should_tokenize_numbers)
{
    char* source = "10 + 30 = 40";
    init_scanner(source);

    Token num_10 = scan_token();
    cr_assert_eq(num_10.type, TOKEN_NUMBER);

    Token plus = scan_token();
    cr_assert_eq(plus.type, TOKEN_PLUS);

    Token num_30 = scan_token();
    cr_assert_eq(num_30.type, TOKEN_NUMBER);

    Token equals = scan_token();
    cr_assert_eq(equals.type, TOKEN_EQUAL);

    Token num_40 = scan_token();
    cr_assert_eq(num_30.type, TOKEN_NUMBER);

    Token eof = scan_token();
    cr_assert_eq(eof.type, TOKEN_EOF);
}

Test(scanner, should_tokenize_identifiers)
{
    char* source = "var user_age = 20;";
    init_scanner(source);

    Token var = scan_token();
    cr_assert_eq(var.type, TOKEN_VAR);
    cr_assert_eq(var.length, strlen("var"));

    Token var_name = scan_token();
    cr_assert_eq(var_name.type, TOKEN_IDENTIFIER);
    cr_assert_eq(var_name.length, strlen("user_age"));

    Token equals = scan_token();
    cr_assert_eq(equals.type, TOKEN_EQUAL);

    Token num_20 = scan_token();
    cr_assert_eq(num_20.type, TOKEN_NUMBER);

    Token semi = scan_token();
    cr_assert_eq(semi.type, TOKEN_SEMICOLON);

    Token eof = scan_token();
    cr_assert_eq(eof.type, TOKEN_EOF);
}

Test(scanner, should_tokenize_identfiers_and_determine_new_lines)
{
    char* source = "for return true false this \n class";
    init_scanner(source);

    Token for_t = scan_token();
    cr_assert_eq(for_t.type, TOKEN_FOR);
    cr_assert_eq(for_t.length, strlen("for"));
    cr_assert_eq(for_t.line, 1);

    Token return_t = scan_token();
    cr_assert_eq(return_t.type, TOKEN_RETURN);
    cr_assert_eq(return_t.length, strlen("return"));
    cr_assert_eq(return_t.line, 1);

    Token true_t = scan_token();
    cr_assert_eq(true_t.type, TOKEN_TRUE);
    cr_assert_eq(true_t.length, strlen("true"));
    cr_assert_eq(true_t.line, 1);

    Token false_t = scan_token();
    cr_assert_eq(false_t.type, TOKEN_FALSE);
    cr_assert_eq(false_t.length, strlen("false"));
    cr_assert_eq(false_t.line, 1);

    Token this_t = scan_token();
    cr_assert_eq(this_t.type, TOKEN_THIS);
    cr_assert_eq(this_t.length, strlen("this"));
    cr_assert_eq(this_t.line, 1);

    Token class_t = scan_token();
    cr_assert_eq(class_t.type, TOKEN_CLASS);
    cr_assert_eq(class_t.length, strlen("class"));
    cr_assert_eq(class_t.line, 2);
}

Test(scanner, tokenize_all_keywords)
{
    TokenTest tests[] = {
        {"and", TOKEN_AND, 3},       {"class", TOKEN_CLASS, 5},
        {"else", TOKEN_ELSE, 4},     {"false", TOKEN_FALSE, 5},
        {"for", TOKEN_FOR, 3},       {"fun", TOKEN_FUN, 3},
        {"if", TOKEN_IF, 2},         {"nil", TOKEN_NIL, 3},
        {"or", TOKEN_OR, 2},         {"print", TOKEN_PRINT, 5},
        {"return", TOKEN_RETURN, 6}, {"super", TOKEN_SUPER, 5},
        {"this", TOKEN_THIS, 4},     {"true", TOKEN_TRUE, 4},
        {"var", TOKEN_VAR, 3},       {"while", TOKEN_WHILE, 5},
    };

    int num_tests = sizeof(tests) / sizeof(TokenTest);

    for (int i = 0; i < num_tests; i++)
    {
        init_scanner(tests[i].input);
        Token token = scan_token();

        cr_assert_eq(token.type, tests[i].expected_type,
                     "Keyword test %d ('%s'): expected type %d, got %d", i,
                     tests[i].input, tests[i].expected_type, token.type);

        cr_assert_eq(token.length, tests[i].expected_length,
                     "Keyword test %d ('%s'): expected length %d, got %d", i,
                     tests[i].input, tests[i].expected_length, token.length);
    }
}

Test(scanner, tokenize_single_char_operators)
{
    TokenTest tests[] = {
        {"(", TOKEN_LEFT_PAREN, 1}, {")", TOKEN_RIGHT_PAREN, 1},
        {"{", TOKEN_LEFT_BRACE, 1}, {"}", TOKEN_RIGHT_BRACE, 1},
        {";", TOKEN_SEMICOLON, 1},  {",", TOKEN_COMMA, 1},
        {".", TOKEN_DOT, 1},        {"-", TOKEN_MINUS, 1},
        {"+", TOKEN_PLUS, 1},       {"/", TOKEN_SLASH, 1},
        {"*", TOKEN_STAR, 1},       {"!", TOKEN_BANG, 1},
        {"=", TOKEN_EQUAL, 1},      {"<", TOKEN_LESS, 1},
        {">", TOKEN_GREATER, 1},
    };

    int num_tests = sizeof(tests) / sizeof(TokenTest);

    for (int i = 0; i < num_tests; i++)
    {
        init_scanner(tests[i].input);
        Token token = scan_token();

        cr_assert_eq(token.type, tests[i].expected_type,
                     "Operator test %d ('%s'): expected type %d, got %d", i,
                     tests[i].input, tests[i].expected_type, token.type);
    }
}

Test(scanner, tokenize_two_char_operators)
{
    TokenTest tests[] = {
        {"!=", TOKEN_BANG_EQUAL, 2},
        {"==", TOKEN_EQUAL_EQUAL, 2},
        {"<=", TOKEN_LESS_EQUAL, 2},
        {">=", TOKEN_GREATER_EQUAL, 2},
    };

    int num_tests = sizeof(tests) / sizeof(TokenTest);

    for (int i = 0; i < num_tests; i++)
    {
        init_scanner(tests[i].input);
        Token token = scan_token();

        cr_assert_eq(token.type, tests[i].expected_type,
                     "Two-char operator test %d ('%s'): expected type "
                     "%d, got %d",
                     i, tests[i].input, tests[i].expected_type, token.type);

        cr_assert_eq(token.length, tests[i].expected_length,
                     "Two-char operator test %d ('%s'): expected length "
                     "%d, got %d",
                     i, tests[i].input, tests[i].expected_length, token.length);
    }
}

Test(scanner, tokenize_various_numbers)
{
    TokenTest tests[] = {
        {"0", TOKEN_NUMBER, 1},       {"123", TOKEN_NUMBER, 3},
        {"999", TOKEN_NUMBER, 3},     {"3.14", TOKEN_NUMBER, 4},
        {"123.456", TOKEN_NUMBER, 7}, {"0.5", TOKEN_NUMBER, 3},
    };

    int num_tests = sizeof(tests) / sizeof(TokenTest);

    for (int i = 0; i < num_tests; i++)
    {
        init_scanner(tests[i].input);
        Token token = scan_token();

        cr_assert_eq(token.type, tests[i].expected_type,
                     "Number test %d ('%s'): expected type %d, got %d", i,
                     tests[i].input, tests[i].expected_type, token.type);

        cr_assert_eq(token.length, tests[i].expected_length,
                     "Number test %d ('%s'): expected length %d, got %d", i,
                     tests[i].input, tests[i].expected_length, token.length);
    }
}

Test(scanner, tokenize_various_identifiers)
{
    TokenTest tests[] = {
        {"x", TOKEN_IDENTIFIER, 1},      {"y", TOKEN_IDENTIFIER, 1},
        {"user", TOKEN_IDENTIFIER, 4},   {"user_age", TOKEN_IDENTIFIER, 8},
        {"myVar", TOKEN_IDENTIFIER, 5},  {"_private", TOKEN_IDENTIFIER, 8},
        {"var123", TOKEN_IDENTIFIER, 6},
    };

    int num_tests = sizeof(tests) / sizeof(TokenTest);

    for (int i = 0; i < num_tests; i++)
    {
        init_scanner(tests[i].input);
        Token token = scan_token();

        cr_assert_eq(token.type, tests[i].expected_type,
                     "Identifier test %d ('%s'): expected type %d, got "
                     "%d",
                     i, tests[i].input, tests[i].expected_type, token.type);

        cr_assert_eq(token.length, tests[i].expected_length,
                     "Identifier test %d ('%s'): expected length %d, got "
                     "%d",
                     i, tests[i].input, tests[i].expected_length, token.length);
    }
}

Test(scanner, should_skip_whitespace)
{
    char* source = "   \t\r  var";
    init_scanner(source);

    Token token = scan_token();
    cr_assert_eq(token.type, TOKEN_VAR);
}

Test(scanner, should_skip_comments)
{
    char* source = "var x // this is a comment\nvar y";
    init_scanner(source);

    Token tok1 = scan_token();
    cr_assert_eq(tok1.type, TOKEN_VAR);

    Token tok2 = scan_token();
    cr_assert_eq(tok2.type, TOKEN_IDENTIFIER);

    Token tok3 = scan_token();
    cr_assert_eq(tok3.type, TOKEN_VAR);
    cr_assert_eq(tok3.line, 2);
}

Test(scanner, should_track_line_numbers)
{
    char* source = "var x\nvar y\nvar z";
    init_scanner(source);

    Token tok1 = scan_token();
    cr_assert_eq(tok1.line, 1);

    Token tok2 = scan_token();
    cr_assert_eq(tok2.line, 1);

    Token tok3 = scan_token();
    cr_assert_eq(tok3.line, 2);

    Token tok4 = scan_token();
    cr_assert_eq(tok4.line, 2);

    Token tok5 = scan_token();
    cr_assert_eq(tok5.line, 3);
}

Test(scanner, should_detect_eof)
{
    char* source = "var";
    init_scanner(source);

    Token tok1 = scan_token();
    cr_assert_eq(tok1.type, TOKEN_VAR);

    Token tok2 = scan_token();
    cr_assert_eq(tok2.type, TOKEN_EOF);
}

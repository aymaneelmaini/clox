#include "../src/scanner.h"
#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <stdio.h>

static inline void print_token(Token token)
{
	printf("Token { type: %d, line: %d, length: %d, start: '%.*s' } \n",
	       token.type, token.line, token.length, token.length, token.start);
}

Test(scanner, should_skip_spaces_and_return_EOF_token)
{
	char *source = "          ";
	init_scanner(source);

	Token result = scan_token();
	cr_assert(result.type == TOKEN_EOF);
}

Test(scanner, should_tokenize_operators)
{
	char *source = "+ - ( ) { } == !=";
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
	char *source = "\"hello world\", \"hello clox\"";
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

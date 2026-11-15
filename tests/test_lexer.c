#include "lexer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST_MAX_TOKENS 32

static void expect_token(
    const Token *token,
    TokenType expected_type,
    const char *expected_text)
{
    size_t expected_length = strlen(expected_text);

    assert(token->type == expected_type);
    assert(token->length == expected_length);
    assert(
        strncmp(
            token->text,
            expected_text,
            expected_length) == 0);
}

static void expect_end(const Token *token)
{
    assert(token->type == TOKEN_END);
    assert(token->text == NULL);
    assert(token->length == 0);
}

static void test_simple_command(void)
{
    char input[] = "ls -l src";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 3);

    expect_token(&tokens[0], TOKEN_WORD, "ls");
    expect_token(&tokens[1], TOKEN_WORD, "-l");
    expect_token(&tokens[2], TOKEN_WORD, "src");
    expect_end(&tokens[3]);
}

static void test_pipeline_without_spaces(void)
{
    char input[] = "ls|grep main";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 4);

    expect_token(&tokens[0], TOKEN_WORD, "ls");
    expect_token(&tokens[1], TOKEN_PIPE, "|");
    expect_token(&tokens[2], TOKEN_WORD, "grep");
    expect_token(&tokens[3], TOKEN_WORD, "main");
    expect_end(&tokens[4]);
}

static void test_append_redirection(void)
{
    char input[] = "echo hello>>output.txt";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 4);

    expect_token(&tokens[0], TOKEN_WORD, "echo");
    expect_token(&tokens[1], TOKEN_WORD, "hello");
    expect_token(
        &tokens[2],
        TOKEN_REDIRECT_APPEND,
        ">>");
    expect_token(
        &tokens[3],
        TOKEN_WORD,
        "output.txt");
    expect_end(&tokens[4]);
}

static void test_and_operator(void)
{
    char input[] = "true&&echo yes";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 4);

    expect_token(&tokens[0], TOKEN_WORD, "true");
    expect_token(&tokens[1], TOKEN_AND_IF, "&&");
    expect_token(&tokens[2], TOKEN_WORD, "echo");
    expect_token(&tokens[3], TOKEN_WORD, "yes");
    expect_end(&tokens[4]);
}

static void test_empty_input(void)
{
    char input[] = "    \t  ";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 0);
    expect_end(&tokens[0]);
}

static void test_stderr_redirections(void)
{
    char input[] = "command 2>error.log 2>&1";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 4);

    expect_token(&tokens[0], TOKEN_WORD, "command");

    expect_token(
        &tokens[1],
        TOKEN_REDIRECT_ERR,
        "2>");

    expect_token(
        &tokens[2],
        TOKEN_WORD,
        "error.log");

    expect_token(
        &tokens[3],
        TOKEN_REDIRECT_ERR_TO_OUT,
        "2>&1");

    expect_end(&tokens[4]);
}

int main(void)
{
    test_simple_command();
    test_pipeline_without_spaces();
    test_append_redirection();
    test_and_operator();
    test_empty_input();
    test_stderr_redirections();

    puts("Lexer tests passed");
    return 0;
}

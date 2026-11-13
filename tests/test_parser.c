#include "lexer.h"
#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST_MAX_TOKENS 256

static void test_simple_command(void)
{
    char input[] = "ls -l src";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 3);

    Command command;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &command,
        &error_index);

    assert(result == PARSER_OK);
    assert(command.argument_count == 3);

    assert(strcmp(command.arguments[0], "ls") == 0);
    assert(strcmp(command.arguments[1], "-l") == 0);
    assert(strcmp(command.arguments[2], "src") == 0);
    assert(command.arguments[3] == NULL);
}

static void test_empty_command(void)
{
    char input[] = "    ";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 0);

    Command command;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &command,
        &error_index);

    assert(result == PARSER_EMPTY);
    assert(command.argument_count == 0);
    assert(command.arguments[0] == NULL);
}

static void test_unexpected_operator(void)
{
    char input[] = "ls|grep main";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 4);

    Command command;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &command,
        &error_index);

    assert(result == PARSER_UNEXPECTED_TOKEN);
    assert(error_index == 1);
    assert(tokens[error_index].type == TOKEN_PIPE);

    /*
     * Parser phải phát hiện lỗi trước khi ghi đè '|'.
     */
    assert(input[2] == '|');
}

static void test_too_many_arguments(void)
{
    Token tokens[SH311_MAX_COMMAND_ARGUMENTS];
    char word[] = "x";

    for (
        size_t i = 0;
        i < SH311_MAX_COMMAND_ARGUMENTS;
        i++)
    {
        tokens[i].type = TOKEN_WORD;
        tokens[i].text = word;
        tokens[i].length = 1;
    }

    Command command;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        SH311_MAX_COMMAND_ARGUMENTS,
        &command,
        &error_index);

    assert(result == PARSER_TOO_MANY_ARGUMENTS);
}

int main(void)
{
    test_simple_command();
    test_empty_command();
    test_unexpected_operator();
    test_too_many_arguments();

    puts("Parser tests passed");
    return 0;
}
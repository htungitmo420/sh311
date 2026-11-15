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

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_OK);
    assert(pipeline.command_count == 1);

    Command *command = &pipeline.commands[0];

    assert(command->argument_count == 3);

    assert(strcmp(command->arguments[0], "ls") == 0);
    assert(strcmp(command->arguments[1], "-l") == 0);
    assert(strcmp(command->arguments[2], "src") == 0);
    assert(command->arguments[3] == NULL);
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

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_EMPTY);
    assert(pipeline.command_count == 0);
}

static void test_pipeline(void)
{
    char input[] = "ls|grep main";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    assert(token_count == 4);

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_OK);
    assert(pipeline.command_count == 2);

    Command *first = &pipeline.commands[0];
    Command *second = &pipeline.commands[1];

    assert(first->argument_count == 1);
    assert(strcmp(first->arguments[0], "ls") == 0);
    assert(first->arguments[1] == NULL);

    assert(second->argument_count == 2);
    assert(strcmp(second->arguments[0], "grep") == 0);
    assert(strcmp(second->arguments[1], "main") == 0);
    assert(second->arguments[2] == NULL);
}

static void test_leading_pipe(void)
{
    char input[] = "| ls";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_EXPECTED_COMMAND);
    assert(error_index == 0);
    assert(tokens[error_index].type == TOKEN_PIPE);
}

static void test_trailing_pipe(void)
{
    char input[] = "ls |";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_EXPECTED_COMMAND);
    assert(error_index == (size_t)token_count);
}

static void test_unexpected_operator(void)
{
    char input[] = "true&&echo yes";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_UNEXPECTED_TOKEN);
    assert(error_index == 1);
    assert(tokens[error_index].type == TOKEN_AND_IF);
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

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        SH311_MAX_COMMAND_ARGUMENTS,
        &pipeline,
        &error_index);

    assert(result == PARSER_TOO_MANY_ARGUMENTS);
}

static void test_output_redirection(void)
{
    char input[] = "echo hello>output.txt";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_OK);
    assert(pipeline.command_count == 1);

    Command *command = &pipeline.commands[0];

    assert(command->argument_count == 2);
    assert(strcmp(command->arguments[0], "echo") == 0);
    assert(strcmp(command->arguments[1], "hello") == 0);
    assert(command->arguments[2] == NULL);

    assert(command->redirection_count == 1);
    assert(command->redirections[0].type == REDIRECTION_OUTPUT);
    assert(strcmp(command->redirections[0].path, "output.txt") == 0);
}

static void test_pipeline_redirections(void)
{
    char input[] = "cat<input.txt|grep hello>>output.txt";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_OK);
    assert(pipeline.command_count == 2);

    Command *first = &pipeline.commands[0];
    Command *second = &pipeline.commands[1];

    assert(first->argument_count == 1);
    assert(strcmp(first->arguments[0], "cat") == 0);
    assert(first->redirection_count == 1);
    assert(first->redirections[0].type == REDIRECTION_INPUT);
    assert(strcmp(first->redirections[0].path, "input.txt") == 0);

    assert(second->argument_count == 2);
    assert(strcmp(second->arguments[0], "grep") == 0);
    assert(strcmp(second->arguments[1], "hello") == 0);
    assert(second->redirection_count == 1);
    assert(second->redirections[0].type == REDIRECTION_APPEND);
    assert(strcmp(second->redirections[0].path, "output.txt") == 0);
}

static void test_missing_redirection_target(void)
{
    char input[] = "echo hello >";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_EXPECTED_REDIRECTION_TARGET);
    assert(error_index == 2);
    assert(tokens[error_index].type == TOKEN_REDIRECT_OUT);
}

static void test_stderr_redirections(void)
{
    char input[] = "command 2>error.log 2>&1";
    Token tokens[TEST_MAX_TOKENS];

    int token_count = lexer_tokenize(
        input,
        tokens,
        TEST_MAX_TOKENS);

    Pipeline pipeline;
    size_t error_index = 0;

    ParserResult result = parser_parse(
        tokens,
        (size_t)token_count,
        &pipeline,
        &error_index);

    assert(result == PARSER_OK);
    assert(pipeline.command_count == 1);

    Command *command = &pipeline.commands[0];

    assert(command->argument_count == 1);
    assert(strcmp(command->arguments[0], "command") == 0);
    assert(command->redirection_count == 2);

    assert(command->redirections[0].type == REDIRECTION_ERROR_OUTPUT);
    assert(strcmp(command->redirections[0].path, "error.log") == 0);

    assert(command->redirections[1].type == REDIRECTION_ERROR_TO_OUTPUT);
    assert(command->redirections[1].path == NULL);
}

int main(void)
{
    test_simple_command();
    test_empty_command();
    test_pipeline();
    test_leading_pipe();
    test_trailing_pipe();
    test_unexpected_operator();
    test_too_many_arguments();
    test_output_redirection();
    test_pipeline_redirections();
    test_missing_redirection_target();
    test_stderr_redirections();

    puts("Parser tests passed");
    return 0;
}

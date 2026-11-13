#define _POSIX_C_SOURCE 200809L
#define MAX_TOKENS 128

#include "exec.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// execute in parent process
static int builtin_cd(char *const arguments[], size_t argument_count)
{
    if (argument_count > 2)
    {
        fprintf(stderr, "sh311: cd: too many arguments\n");
        return 1;
    }

    const char *directory;

    if (argument_count == 1)
    {
        directory = getenv("HOME");

        if (directory == NULL)
        {
            fprintf(stderr, "sh311: cd: HOME is not set\n");
            return 1;
        }
    }
    else
    {
        directory = arguments[1];
    }

    if (chdir(directory) < 0)
    {
        perror("sh311: cd");
        return 1;
    }

    return 0;
}

int main(void)
{
    char *line = NULL;
    size_t capacity = 0;
    int last_status = 0;
    const int interactive = isatty(STDIN_FILENO);

    while (1)
    {
        if (interactive)
        {
            printf("sh311$ ");
            fflush(stdout);
        }

        ssize_t characters_read = getline(&line, &capacity, stdin);

        if (characters_read == -1)
        {
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        Token tokens[MAX_TOKENS];

        int token_count = lexer_tokenize(line, tokens, MAX_TOKENS);

        if (token_count < 0)
        {
            fprintf(stderr, "sh311: lexer: too many tokens\n");
            last_status = 2;
            continue;
        }

        if (token_count == 0)
        {
            continue;
        }

        Pipeline pipeline;
        size_t error_index = 0;

        ParserResult parser_result = parser_parse(tokens, (size_t)token_count, &pipeline, &error_index);

        if (parser_result == PARSER_EMPTY)
        {
            continue;
        }

        if (parser_result == PARSER_UNEXPECTED_TOKEN)
        {
            fprintf(stderr, "sh311: unexpected token '%s'\n", lexer_token_type_name(tokens[error_index].type));
            last_status = 2;
            continue;
        }

        if (parser_result == PARSER_EXPECTED_COMMAND)
        {
            if (error_index == (size_t)token_count)
            {
                fprintf(stderr, "sh311: expected command after '|'\n");
            }
            else
            {
                fprintf(
                    stderr,
                    "sh311: expected command near '%s'\n",
                    lexer_token_type_name(tokens[error_index].type));
            }

            last_status = 2;
            continue;
        }

        if (parser_result == PARSER_TOO_MANY_ARGUMENTS)
        {
            fprintf(stderr, "sh311: too many command arguments\n");
            last_status = 2;
            continue;
        }

        if (parser_result == PARSER_TOO_MANY_COMMANDS)
        {
            fprintf(stderr, "sh311: too many pipeline commands\n");
            last_status = 2;
            continue;
        }

        if (pipeline.command_count > 1)
        {
            fprintf(stderr, "sh311: pipeline execution is not implemented yet\n");
            last_status = 2;
            continue;
        }

        Command *command = &pipeline.commands[0];

        char **arguments = command->arguments;
        size_t argument_count = command->argument_count;

        if (strcmp(arguments[0], "cd") == 0)
        {
            last_status = builtin_cd(arguments, argument_count);
            continue;
        }

        if (strcmp(arguments[0], "exit") == 0)
        {
            break;
        }

        last_status = executor_run_command(command);
    }

    free(line);
    return last_status;
}

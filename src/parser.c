#include "parser.h"

static void initialize_command(Command *command)
{
    command->argument_count = 0;
    command->arguments[0] = NULL;
}

ParserResult parser_parse(
    Token tokens[],
    size_t token_count,
    Pipeline *pipeline,
    size_t *error_index)
{
    pipeline->command_count = 0;

    if (error_index != NULL)
    {
        *error_index = 0;
    }

    if (token_count == 0)
    {
        return PARSER_EMPTY;
    }

    size_t command_count = 1;
    size_t argument_count = 0;

    for (size_t i = 0; i < token_count; i++)
    {
        if (tokens[i].type == TOKEN_WORD)
        {
            if (
                argument_count + 1 >=
                SH311_MAX_COMMAND_ARGUMENTS)
            {
                if (error_index != NULL)
                {
                    *error_index = i;
                }

                return PARSER_TOO_MANY_ARGUMENTS;
            }

            argument_count++;
            continue;
        }

        if (tokens[i].type == TOKEN_PIPE)
        {
            if (argument_count == 0)
            {
                if (error_index != NULL)
                {
                    *error_index = i;
                }

                return PARSER_EXPECTED_COMMAND;
            }

            if (
                command_count >=
                SH311_MAX_PIPELINE_COMMANDS)
            {
                if (error_index != NULL)
                {
                    *error_index = i;
                }

                return PARSER_TOO_MANY_COMMANDS;
            }

            command_count++;
            argument_count = 0;
            continue;
        }

        if (error_index != NULL)
        {
            *error_index = i;
        }

        return PARSER_UNEXPECTED_TOKEN;
    }

    if (argument_count == 0)
    {
        if (error_index != NULL)
        {
            *error_index = token_count;
        }

        return PARSER_EXPECTED_COMMAND;
    }

    size_t command_index = 0;
    Command *command = &pipeline->commands[command_index];

    initialize_command(command);

    for (size_t i = 0; i < token_count; i++)
    {
        if (tokens[i].type == TOKEN_PIPE)
        {
            command->arguments[command->argument_count] = NULL;

            command_index++;
            command = &pipeline->commands[command_index];
            initialize_command(command);

            continue;
        }

        tokens[i].text[tokens[i].length] = '\0';

        command->arguments[command->argument_count] =
            tokens[i].text;

        command->argument_count++;
    }

    command->arguments[command->argument_count] = NULL;
    pipeline->command_count = command_count;

    return PARSER_OK;
}
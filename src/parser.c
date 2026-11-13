#include "parser.h"

ParserResult parser_parse(Token tokens[], size_t token_count, Command *command, size_t *error_index)
{
    command->argument_count = 0;
    command->arguments[0] = NULL;

    if (error_index != NULL)
    {
        *error_index = 0;
    }

    if (token_count == 0)
    {
        return PARSER_EMPTY;
    }

    for (size_t i = 0; i < token_count; i++)
    {
        if (tokens[i].type != TOKEN_WORD)
        {
            if (error_index != NULL)
            {
                *error_index = i;
            }

            return PARSER_UNEXPECTED_TOKEN;
        }
    }

    if (token_count >= SH311_MAX_COMMAND_ARGUMENTS)
    {
        return PARSER_TOO_MANY_ARGUMENTS;
    }

    for (size_t i = 0; i < token_count; i++)
    {
        tokens[i].text[tokens[i].length] = '\0';

        command->arguments[i] = tokens[i].text;
        command->argument_count++;
    }

    command->arguments[command->argument_count] = NULL;

    return PARSER_OK;
}
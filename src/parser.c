#include "parser.h"

static void initialize_command(Command *command)
{
    command->argument_count = 0;
    command->arguments[0] = NULL;

    command->redirection_count = 0;
}

static int is_redirection_token(TokenType type)
{
    return type == TOKEN_REDIRECT_IN ||
           type == TOKEN_REDIRECT_OUT ||
           type == TOKEN_REDIRECT_APPEND ||
           type == TOKEN_REDIRECT_ERR ||
           type == TOKEN_REDIRECT_ERR_TO_OUT;
}

static int redirection_requires_path(TokenType type)
{
    return type != TOKEN_REDIRECT_ERR_TO_OUT;
}

static RedirectionType redirection_type_from_token(TokenType type)
{
    if (type == TOKEN_REDIRECT_IN)
    {
        return REDIRECTION_INPUT;
    }

    if (type == TOKEN_REDIRECT_APPEND)
    {
        return REDIRECTION_APPEND;
    }

    if (type == TOKEN_REDIRECT_ERR)
    {
        return REDIRECTION_ERROR_OUTPUT;
    }

    if (type == TOKEN_REDIRECT_ERR_TO_OUT)
    {
        return REDIRECTION_ERROR_TO_OUTPUT;
    }

    return REDIRECTION_OUTPUT;
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
    size_t redirection_count = 0;

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

        if (is_redirection_token(tokens[i].type))
        {
            if (redirection_count >= SH311_MAX_REDIRECTIONS)
            {
                if (error_index != NULL)
                {
                    *error_index = i;
                }

                return PARSER_TOO_MANY_REDIRECTIONS;
            }

            redirection_count++;

            if (!redirection_requires_path(tokens[i].type))
            {
                continue;
            }

            size_t target_index = i + 1;

            if (target_index >= token_count || tokens[target_index].type != TOKEN_WORD)
            {
                if (error_index != NULL)
                {
                    *error_index = i;
                }

                return PARSER_EXPECTED_REDIRECTION_TARGET;
            }
            i = target_index;
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
            redirection_count = 0;
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

        if (is_redirection_token(tokens[i].type))
        {
            Redirection *redirection =
                &command->redirections[command->redirection_count];

            redirection->type =
                redirection_type_from_token(tokens[i].type);

            if (!redirection_requires_path(tokens[i].type))
            {
                redirection->path = NULL;
                command->redirection_count++;
                continue;
            }

            Token *path_token = &tokens[i + 1];

            path_token->text[path_token->length] = '\0';
            redirection->path = path_token->text;

            command->redirection_count++;

            i++;
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

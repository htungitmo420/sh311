#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#include <stddef.h>

#define SH311_MAX_COMMAND_ARGUMENTS 128
#define SH311_MAX_PIPELINE_COMMANDS 32
#define SH311_MAX_REDIRECTIONS 16

typedef enum
{
    PARSER_OK,
    PARSER_EMPTY,
    PARSER_UNEXPECTED_TOKEN,
    PARSER_EXPECTED_COMMAND,
    PARSER_EXPECTED_REDIRECTION_TARGET,
    PARSER_TOO_MANY_ARGUMENTS,
    PARSER_TOO_MANY_COMMANDS,
    PARSER_TOO_MANY_REDIRECTIONS
} ParserResult;

typedef enum
{
    REDIRECTION_INPUT,
    REDIRECTION_OUTPUT,
    REDIRECTION_APPEND,
    REDIRECTION_ERROR_OUTPUT,
    REDIRECTION_ERROR_TO_OUTPUT
} RedirectionType;

typedef struct
{
    RedirectionType type;
    char *path;
} Redirection;

typedef struct
{
    char *arguments[SH311_MAX_COMMAND_ARGUMENTS];
    size_t argument_count;

    Redirection redirections[SH311_MAX_REDIRECTIONS];
    size_t redirection_count;
} Command;

typedef struct
{
    Command commands[SH311_MAX_PIPELINE_COMMANDS];
    size_t command_count;
} Pipeline;

ParserResult
parser_parse(Token tokens[], size_t token_count, Pipeline *pipeline, size_t *error_index);

#endif

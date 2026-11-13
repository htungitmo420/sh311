#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#include <stddef.h>

#define SH311_MAX_COMMAND_ARGUMENTS 128
#define SH311_MAX_PIPELINE_COMMANDS 32

typedef struct
{
    char *arguments[SH311_MAX_COMMAND_ARGUMENTS];
    size_t argument_count;
} Command;

typedef struct
{
    Command commands[SH311_MAX_PIPELINE_COMMANDS];
    size_t command_count;
} Pipeline;

typedef enum
{
    PARSER_OK,
    PARSER_EMPTY,
    PARSER_UNEXPECTED_TOKEN,
    PARSER_EXPECTED_COMMAND,
    PARSER_TOO_MANY_ARGUMENTS,
    PARSER_TOO_MANY_COMMANDS
} ParserResult;

ParserResult parser_parse(Token tokens[], size_t token_count, Pipeline *pipeline, size_t *error_index);

#endif
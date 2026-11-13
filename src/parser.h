#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#include <stddef.h>

#define SH311_MAX_COMMAND_ARGUMENTS 128

typedef struct
{
    char *arguments[SH311_MAX_COMMAND_ARGUMENTS];
    size_t argument_count;
} Command;

typedef enum
{
    PARSER_OK,
    PARSER_EMPTY,
    PARSER_UNEXPECTED_TOKEN,
    PARSER_TOO_MANY_ARGUMENTS
} ParserResult;

ParserResult parser_parse(Token tokens[], size_t token_count, Command *command, size_t *error_index);

#endif
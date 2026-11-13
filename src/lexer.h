#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum
{
    TOKEN_WORLD,
    TOKEN_END
} TokenType;

typedef struct
{
    TokenType type;
    char *text;
} Token;

int lexer_tokenize(char *line, Token tokens[], size_t capacity);

#endif
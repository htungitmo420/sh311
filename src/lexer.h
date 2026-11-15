#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_REDIRECT_APPEND,
    TOKEN_AND_IF,
    TOKEN_OR_IF,
    TOKEN_SEMICOLON,
    TOKEN_BACKGROUND,
    TOKEN_REDIRECT_ERR,
    TOKEN_REDIRECT_ERR_TO_OUT,
    TOKEN_END
} TokenType;

typedef struct
{
    TokenType type;
    char *text;
    size_t length;
} Token;

int lexer_tokenize(char *line, Token tokens[], size_t capacity);
const char *lexer_token_type_name(TokenType type);

#endif
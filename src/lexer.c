#define _POSIX_C_SOURCE 200809L

#include "lexer.h"

#include <string.h>

int lexer_tokenize(char *line, Token tokens[], size_t capacity)
{
    if (capacity == 0)
    {
        return -1;
    }

    size_t token_count = 0;
    char *save_pointer = NULL;
    char *word = strtok_r(line, " \t", &save_pointer);

    while (word != NULL)
    {
        if (token_count + 1 >= capacity)
        {
            return -1;
        }

        tokens[token_count].type = TOKEN_WORLD;
        tokens[token_count].text = word;
        token_count++;

        word = strtok_r(NULL, " \t", &save_pointer);
    }

    tokens[token_count].type = TOKEN_END;
    tokens[token_count].text = NULL;

    return (int)token_count;
}
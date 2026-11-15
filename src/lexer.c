#define _POSIX_C_SOURCE 200809L

#include "lexer.h"

#include <ctype.h>

static int is_operator_start(char character)
{
    return character == '|' ||
           character == '<' ||
           character == '>' ||
           character == ';' ||
           character == '&';
}

int lexer_tokenize(char *line, Token tokens[], size_t capacity)
{
    if (capacity == 0)
    {
        return -1;
    }

    size_t token_count = 0;
    char *cursor = line;

    while (1)
    {
        while (isspace((unsigned char)*cursor))
        {
            cursor++;
        }

        if (*cursor == '\0')
        {
            break;
        }

        if (token_count + 1 >= capacity)
        {
            return -1;
        }

        if (cursor[0] == '2' &&
            cursor[1] == '>' &&
            cursor[2] == '&' &&
            cursor[3] == '1')
        {
            tokens[token_count].type = TOKEN_REDIRECT_ERR_TO_OUT;
            tokens[token_count].text = cursor;
            tokens[token_count].length = 4;

            token_count++;
            cursor += 4;
            continue;
        }

        if (cursor[0] == '2' &&
            cursor[1] == '>')
        {
            tokens[token_count].type = TOKEN_REDIRECT_ERR;
            tokens[token_count].text = cursor;
            tokens[token_count].length = 2;

            token_count++;
            cursor += 2;
            continue;
        }

        if (!is_operator_start(*cursor))
        {
            char *word_start = cursor;

            while (*cursor != '\0' && !isspace((unsigned char)*cursor) && !is_operator_start(*cursor))
            {
                cursor++;
            }

            tokens[token_count].type = TOKEN_WORD;
            tokens[token_count].text = word_start;
            tokens[token_count].length = (size_t)(cursor - word_start);

            token_count++;
            continue;
        }

        TokenType operator_type;
        size_t operator_length = 1;

        switch (*cursor)
        {
        case '|':
            if (cursor[1] == '|')
            {
                operator_type = TOKEN_OR_IF;
                operator_length = 2;
            }
            else
            {
                operator_type = TOKEN_PIPE;
            }
            break;

        case '&':
            if (cursor[1] == '&')
            {
                operator_type = TOKEN_AND_IF;
                operator_length = 2;
            }
            else
            {
                operator_type = TOKEN_BACKGROUND;
            }
            break;

        case '>':
            if (cursor[1] == '>')
            {
                operator_type = TOKEN_REDIRECT_APPEND;
                operator_length = 2;
            }
            else
            {
                operator_type = TOKEN_REDIRECT_OUT;
            }
            break;

        case '<':
            operator_type = TOKEN_REDIRECT_IN;
            break;

        case ';':
            operator_type = TOKEN_SEMICOLON;
            break;

        default:
            return -1;
        }

        tokens[token_count].type = operator_type;
        tokens[token_count].text = cursor;
        tokens[token_count].length = operator_length;

        token_count++;
        cursor += operator_length;
    }

    tokens[token_count].type = TOKEN_END;
    tokens[token_count].text = NULL;
    tokens[token_count].length = 0;

    return (int)token_count;
}

const char *lexer_token_type_name(TokenType type)
{
    switch (type)
    {
    case TOKEN_WORD:
        return "word";
    case TOKEN_PIPE:
        return "|";
    case TOKEN_REDIRECT_IN:
        return "<";
    case TOKEN_REDIRECT_OUT:
        return ">";
    case TOKEN_REDIRECT_APPEND:
        return ">>";
    case TOKEN_AND_IF:
        return "&&";
    case TOKEN_OR_IF:
        return "||";
    case TOKEN_SEMICOLON:
        return ";";
    case TOKEN_BACKGROUND:
        return "&";
    case TOKEN_REDIRECT_ERR:
        return "2>";
    case TOKEN_REDIRECT_ERR_TO_OUT:
        return "2>&1";
    case TOKEN_END:
        return "end";
    }

    return "unknown";
}

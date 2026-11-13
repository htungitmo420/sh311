#define _POSIX_C_SOURCE 200809L
#define MAX_TOKENS 128

#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// execute in parent process
static int builtin_cd(char *const arguments[], size_t argument_count)
{
    if (argument_count > 2)
    {
        fprintf(stderr, "sh311: cd: too many arguments\n");
        return 1;
    }

    const char *directory;

    if (argument_count == 1)
    {
        directory = getenv("HOME");

        if (directory == NULL)
        {
            fprintf(stderr, "sh311: cd: HOME is not set\n");
            return 1;
        }
    }
    else
    {
        directory = arguments[1];
    }

    if (chdir(directory) < 0)
    {
        perror("sh311: cd");
        return 1;
    }

    return 0;
}

// execute in child process
static int run_external_command(char *const arguments[])
{
    pid_t child_pid = fork();

    if (child_pid < 0)
    {
        perror("sh311: fork");
        return 1;
    }

    if (child_pid == 0)
    {
        execvp(arguments[0], arguments);
        perror(arguments[0]);
        _exit(127);
    }

    int child_status;

    if (waitpid(child_pid, &child_status, 0) < 0)
    {
        perror("perror: waitpid");
        return 1;
    }

    if (WIFEXITED(child_status))
    {
        return WEXITSTATUS(child_status);
    }

    if (WIFSIGNALED(child_status))
    {
        return 128 + WTERMSIG(child_status);
    }

    return 1;
}

int main(void)
{
    char *line = NULL;
    size_t capacity = 0;
    int last_status = 0;
    const int interactive = isatty(STDIN_FILENO);

    while (1)
    {
        if (interactive)
        {
            printf("sh311$ ");
            fflush(stdout);
        }

        ssize_t characters_read = getline(&line, &capacity, stdin);

        if (characters_read == -1)
        {
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        Token tokens[MAX_TOKENS];

        int token_count = lexer_tokenize(line, tokens, MAX_TOKENS);

        if (token_count < 0)
        {
            fprintf(stderr, "sh311: lexer: too many tokens\n");
            last_status = 2;
            continue;
        }

        if (token_count == 0)
        {
            continue;
        }

        int contains_operator = 0;

        for (int i = 0; i < token_count; i++)
        {
            if (tokens[i].type != TOKEN_WORD)
            {
                fprintf(stderr, "sh311: parser not implemented for token '%s\n", lexer_token_type_name(tokens[i].type));
                contains_operator = 1;
                last_status = 2;
                break;
            }
        }

        if (contains_operator)
        {
            continue;
        }

        char *arguments[MAX_TOKENS];

        for (int i = 0; i < token_count; i++)
        {
            tokens[i].text[tokens[i].length] = '\0';
            arguments[i] = tokens[i].text;
        }

        arguments[token_count] = NULL;

        size_t argument_count = (size_t)token_count;

        if (argument_count == 0)
        {
            continue;
        }

        if (strcmp(arguments[0], "cd") == 0)
        {
            last_status = builtin_cd(arguments, argument_count);
            continue;
        }

        if (strcmp(arguments[0], "exit") == 0)
        {
            break;
        }

        last_status = run_external_command(arguments);
    }

    free(line);
    return last_status;
}
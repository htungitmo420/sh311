#define _POSIX_C_SOURCE 200809L
#define MAX_ARGUMENTS 128

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
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

static int split_line(
    char *line,
    char *arguments[],
    size_t capacity)
{
    char *save_pointer = NULL;
    char *token = strtok_r(line, " \t", &save_pointer);
    size_t argument_count = 0;

    while (token != NULL)
    {
        if (argument_count + 1 >= capacity)
        {
            fprintf(stderr, "sh311: too many arguments; maximum is %zu\n", capacity - 1);
            return -1;
        }

        arguments[argument_count] = token;
        argument_count++;

        token = strtok_r(NULL, " \t", &save_pointer);
    }

    arguments[argument_count] = NULL;
    return (int)argument_count;
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

        char *arguments[MAX_ARGUMENTS];

        int argument_count = split_line(line, arguments, MAX_ARGUMENTS);

        if (argument_count < 0)
        {
            last_status = 2;
            continue;
        }

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
#include "exec.h"

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int executor_run_command(const Command *command)
{
    if (command == NULL || command->argument_count == 0 || command->arguments[0] == NULL)
    {
        fprintf(stderr, "sh311: cannot execute empty command\n");
        return 1;
    }

    pid_t child_pid = fork();

    if (child_pid < 0)
    {
        perror("sh311: fork");
        return 1;
    }

    if (child_pid == 0)
    {
        execvp(command->arguments[0], command->arguments);
        perror(command->arguments[0]);
        _exit(127);
    }

    int child_status = 0;

    while (waitpid(child_pid, &child_status, 0) < 0)
    {
        if (errno == EINTR)
        {
            continue;
        }

        perror("sh311: waitpid");
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
#include "exec.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

static int wait_for_process(pid_t pid, int *status)
{
    while (waitpid(pid, status, 0) < 0)
    {
        if (errno == EINTR)
        {
            continue;
        }

        perror("sh311: waitpid");
        return -1;
    }

    return 0;
}

static int status_to_exit_code(int status)
{
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status))
    {
        return 128 + WTERMSIG(status);
    }

    return 1;
}

static int apply_redirections(const Command *command)
{
    for (size_t i = 0; i < command->redirection_count; i++)
    {
        const Redirection *redirection = &command->redirections[i];

        int descriptor;
        int target_descriptor;

        switch (redirection->type)
        {
        case REDIRECTION_INPUT:
            descriptor = open(redirection->path, O_RDONLY);
            target_descriptor = STDIN_FILENO;
            break;

        case REDIRECTION_OUTPUT:
            descriptor = open(redirection->path,
                              O_WRONLY | O_CREAT | O_TRUNC, 0666);
            target_descriptor = STDOUT_FILENO;
            break;

        case REDIRECTION_APPEND:
            descriptor = open(redirection->path, O_WRONLY | O_CREAT | O_APPEND, 0666);
            target_descriptor = STDOUT_FILENO;
            break;

        case REDIRECTION_ERROR_OUTPUT:
            descriptor = open(redirection->path,
                              O_WRONLY | O_CREAT | O_TRUNC, 0666);
            target_descriptor = STDERR_FILENO;
            break;

        case REDIRECTION_ERROR_TO_OUTPUT:
            if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0)
            {
                perror("sh311: dup2 stderr");
                return -1;
            }
            continue;

        default:
            fprintf(stderr, "sh311: unknown redirection type\n");
            return -1;
        }

        if (descriptor < 0)
        {
            fprintf(stderr, "sh311: %s: %s\n", redirection->path, strerror(errno));
            return -1;
        }

        if (descriptor != target_descriptor)
        {
            if (dup2(descriptor, target_descriptor) < 0)
            {
                perror("sh311: dup2 redirection");
                close(descriptor);
                return -1;
            }

            close(descriptor);
        }
    }

    return 0;
}

static void execute_child(
    const Command *command,
    int previous_read,
    int next_pipe[2],
    int has_next_command)
{
    if (previous_read >= 0)
    {
        if (dup2(previous_read, STDIN_FILENO) < 0)
        {
            perror("sh311: dup2 stdin");
            _exit(1);
        }
    }

    if (has_next_command)
    {
        if (dup2(next_pipe[1], STDOUT_FILENO) < 0)
        {
            perror("sh311: dup2 stdout");
            _exit(1);
        }
    }

    if (previous_read >= 0)
    {
        close(previous_read);
    }

    if (has_next_command)
    {
        close(next_pipe[0]);
        close(next_pipe[1]);
    }

    if (apply_redirections(command) < 0)
    {
        _exit(1);
    }

    execvp(
        command->arguments[0],
        command->arguments);

    int exec_error = errno;

    fprintf(
        stderr,
        "sh311: %s: %s\n",
        command->arguments[0],
        strerror(exec_error));

    _exit(exec_error == ENOENT ? 127 : 126);
}

int executor_run_pipeline(const Pipeline *pipeline)
{
    if (
        pipeline == NULL ||
        pipeline->command_count == 0)
    {
        fprintf(stderr, "sh311: cannot execute empty pipeline\n");
        return 1;
    }

    pid_t process_ids[SH311_MAX_PIPELINE_COMMANDS];
    size_t process_count = 0;
    int previous_read = -1;

    for (
        size_t i = 0;
        i < pipeline->command_count;
        i++)
    {
        int has_next_command =
            i + 1 < pipeline->command_count;

        int next_pipe[2] = {-1, -1};

        if (has_next_command)
        {
            if (pipe(next_pipe) < 0)
            {
                perror("sh311: pipe");

                if (previous_read >= 0)
                {
                    close(previous_read);
                }

                for (size_t j = 0; j < process_count; j++)
                {
                    int ignored_status;
                    wait_for_process(
                        process_ids[j],
                        &ignored_status);
                }

                return 1;
            }
        }

        pid_t child_pid = fork();

        if (child_pid < 0)
        {
            perror("sh311: fork");

            if (previous_read >= 0)
            {
                close(previous_read);
            }

            if (has_next_command)
            {
                close(next_pipe[0]);
                close(next_pipe[1]);
            }

            for (size_t j = 0; j < process_count; j++)
            {
                int ignored_status;
                wait_for_process(
                    process_ids[j],
                    &ignored_status);
            }

            return 1;
        }

        if (child_pid == 0)
        {
            execute_child(
                &pipeline->commands[i],
                previous_read,
                next_pipe,
                has_next_command);
        }

        process_ids[process_count] = child_pid;
        process_count++;

        if (previous_read >= 0)
        {
            close(previous_read);
        }

        if (has_next_command)
        {

            close(next_pipe[1]);
            previous_read = next_pipe[0];
        }
        else
        {
            previous_read = -1;
        }
    }

    int pipeline_exit_code = 1;
    int wait_failed = 0;

    for (size_t i = 0; i < process_count; i++)
    {
        int child_status = 0;

        if (
            wait_for_process(
                process_ids[i],
                &child_status) < 0)
        {
            wait_failed = 1;
            continue;
        }

        if (i + 1 == process_count)
        {
            pipeline_exit_code =
                status_to_exit_code(child_status);
        }
    }

    if (wait_failed)
    {
        return 1;
    }

    return pipeline_exit_code;
}

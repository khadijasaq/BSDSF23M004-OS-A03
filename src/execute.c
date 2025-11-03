
#include "shell.h"
#include <fcntl.h>      // <-- Add this line
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int execute(char* arglist[]) {
    int pipefd[2];
    int in_redirect = 0, out_redirect = 0, pipe_present = 0;
    char *input_file = NULL, *output_file = NULL;
    char *cmd1[MAXARGS], *cmd2[MAXARGS];

    // Parse for redirection and pipe
    int i = 0, j = 0, k = 0;
    while (arglist[i] != NULL) {
        if (strcmp(arglist[i], "<") == 0) {
            in_redirect = 1;
            input_file = arglist[++i];
        } 
        else if (strcmp(arglist[i], ">") == 0) {
            out_redirect = 1;
            output_file = arglist[++i];
        } 
        else if (strcmp(arglist[i], "|") == 0) {
            pipe_present = 1;
            arglist[i] = NULL;
            j = 0;
            i++;
            while (arglist[i] != NULL) {
                cmd2[j++] = arglist[i++];
            }
            cmd2[j] = NULL;
            break;
        } 
        else {
            cmd1[k++] = arglist[i];
        }
        i++;
    }
    cmd1[k] = NULL;

    // Handle pipe
    if (pipe_present) {
        if (pipe(pipefd) == -1) {
            perror("pipe failed");
            return 1;
        }

        pid_t pid1 = fork();
        if (pid1 == 0) { // Left child
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            execvp(cmd1[0], cmd1);
            perror("Command failed");
            exit(1);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) { // Right child
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            execvp(cmd2[0], cmd2);
            perror("Command failed");
            exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return 0;
    }

    // Normal command (with optional redirection)
    pid_t pid = fork();

    if (pid == 0) {
        if (in_redirect) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("input file error");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (out_redirect) {
            int fd_out = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("output file error");
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(cmd1[0], cmd1);
        perror("Command failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        perror("fork failed");
    }

    return 0;
}

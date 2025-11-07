#include "shell.h"

#include "shell.h"

#define MAX_CMDS 10
#define MAX_JOBS 20

// declare globals so main and shell share them
pid_t bg_jobs[MAX_JOBS];
int job_count = 0;

int main() {
    char *cmdline;
    char **arglist;
    char *commands[MAX_CMDS];

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // split multiple commands separated by ';'
        int cmd_count = 0;
        char *token = strtok(cmdline, ";");
        while (token != NULL && cmd_count < MAX_CMDS) {
            commands[cmd_count++] = strdup(token);
            token = strtok(NULL, ";");
        }

        // reap finished background jobs
        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            printf("[+] Process %d finished\n", pid);
        }

        for (int i = 0; i < cmd_count; i++) {
            char *single_cmd = commands[i];
            char *trim = strtok(single_cmd, "\n");
            if (trim == NULL || strlen(trim) == 0)
                continue;

            int background = 0;
            if (trim[strlen(trim) - 1] == '&') {
                background = 1;
                trim[strlen(trim) - 1] = '\0';   // remove '&'
            }

            arglist = tokenize(trim);
            if (arglist == NULL) continue;

            if (!handle_builtin(arglist)) {
                pid_t pid = fork();
                if (pid == 0) {
                    execute(arglist);
                    exit(0);
                } else if (pid > 0) {
                    if (background) {
                        printf("[bg] process started with PID %d\n", pid);
                        if (job_count < MAX_JOBS)
                            bg_jobs[job_count++] = pid;
                    } else {
                        waitpid(pid, NULL, 0);
                    }
                } else {
                    perror("fork failed");
                }
            }

            for (int j = 0; arglist[j] != NULL; j++)
                free(arglist[j]);
            free(arglist);
            free(commands[i]);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

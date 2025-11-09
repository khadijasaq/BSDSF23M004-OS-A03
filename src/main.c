#include "shell.h"

#include "shell.h"

#define MAX_CMDS 10
#define MAX_JOBS 20
/*int run_command(char *cmdline) {
    char **arglist = tokenize(cmdline);
    if (arglist == NULL) return 1; // Treat empty as failure

    pid_t pid = fork();
    if (pid == 0) {
        execute(arglist);
        exit(errno != 0); // Nonzero = failure
    } else {
        int status;
        waitpid(pid, &status, 0);
        for (int i = 0; arglist[i] != NULL; i++)
            free(arglist[i]);
        free(arglist);
        return WEXITSTATUS(status);
    }
}*/
void expand_env_vars(char *cmdline) {
    char temp[1024] = "";
    for (int i = 0; cmdline[i] != '\0';) {
        if (cmdline[i] == '$') {
            i++;
            char var[128];
            int j = 0;

            // capture variable name
            while ((cmdline[i] >= 'A' && cmdline[i] <= 'Z') ||
                   (cmdline[i] >= 'a' && cmdline[i] <= 'z') ||
                   (cmdline[i] >= '0' && cmdline[i] <= '9') ||
                   cmdline[i] == '_') {
                var[j++] = cmdline[i++];
            }
            var[j] = '\0';

            const char *val = getenv(var);
            if (val)
                strcat(temp, val);   // substitute value
        } else {
            strncat(temp, &cmdline[i], 1);
            i++;
        }
    }
    strcpy(cmdline, temp);
}

          void execute_lines(const char *body) {
    if (body == NULL || strlen(body) == 0)
        return;

    char *copy = strdup(body);
    if (!copy) return;

    char *cmd = strtok(copy, "\n");
    while (cmd != NULL) {
        if (strlen(cmd) > 0) {
            // skip pure whitespace
            while (*cmd == ' ' || *cmd == '\t') cmd++;
            if (strlen(cmd) == 0) {
                cmd = strtok(NULL, "\n");
                continue;
            }

            char **arglist = tokenize(cmd);
            if (arglist) {
                if (!handle_builtin(arglist)) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        execute(arglist);
                        exit(0);
                    } else {
                        waitpid(pid, NULL, 0);
                    }
                }
                for (int j = 0; arglist[j] != NULL; j++)
                    free(arglist[j]);
                free(arglist);
            }
        }
        cmd = strtok(NULL, "\n");
    }
    free(copy);
}
 
int evaluate_condition(const char *expr) {
    // Example: "[ $i -le 3 ]"
    char var[64], op[4];
    int val, var_val;
    sscanf(expr, "[ $%63s %3s %d ]", var, op, &val);
    var_val = atoi(getenv(var)); // get current variable value
    if (strcmp(op, "-le") == 0) return var_val <= val;
    if (strcmp(op, "-lt") == 0) return var_val < val;
    if (strcmp(op, "-eq") == 0) return var_val == val;
    return 0;
}

int run_command(char *cmdline) {
    if (cmdline == NULL || strlen(cmdline) == 0)
        return 1;

    pid_t pid = fork();
    if (pid == 0) {
        execlp("bash", "bash", "-c", cmdline, (char *)NULL);
        perror("execlp failed");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}


// declare globals so main and shell share them
pid_t bg_jobs[MAX_JOBS];
int job_count = 0;

int main() {
    char *cmdline;
    char **arglist;
    char *commands[MAX_CMDS];

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
	if (strncmp(cmdline, "if ", 3) == 0) {
    char condition[256], then_block[512] = "", else_block[512] = "";
    char line[MAX_LEN];

    // Extract condition
    sscanf(cmdline + 3, "%[^\n]", condition);

    // Read until 'fi'
    int in_then = 0, in_else = 0;
    while (1) {
        printf("if> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        if (strncmp(line, "then", 4) == 0) {
            in_then = 1;
            continue;
        } else if (strncmp(line, "else", 4) == 0) {
            in_else = 1;
            in_then = 0;
            continue;
        } else if (strncmp(line, "fi", 2) == 0) {
            break;
        }

        if (in_then)
            strcat(then_block, line);
        else if (in_else)
            strcat(else_block, line);
    }

    // Evaluate condition
    int status = run_command(condition);
    if (status == 0)
        run_command(then_block);
    else
        run_command(else_block);

    free(cmdline);
    continue;
}


if (strncmp(cmdline, "while ", 6) == 0) {
    char condition[512];
    sscanf(cmdline + 6, "%[^\n]", condition);

    // read loop body
    char body[2048] = "";
    char line[MAX_LEN];
    while (1) {
        printf("while> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, "do") == 0) continue;
        if (strcmp(line, "done") == 0) break;
        strcat(body, line);
        strcat(body, "\n");
    }

    // interpret it internally
    while (evaluate_condition(condition)) {
        execute_lines(body);
    }

    free(cmdline);
    continue;
}
  



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
expand_env_vars(trim);
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

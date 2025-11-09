#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#define MAX_JOBS 20

#define MAX_VARS 50
static char *var_names[MAX_VARS];
static char *var_values[MAX_VARS];
static int var_count = 0;

static pid_t bg_jobs[MAX_JOBS];
static int job_count = 0;

static char* history[HISTORY_SIZE];
static int history_count = 0;


void set_var(char *name, char *value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_names[i], name) == 0) {
            free(var_values[i]);
            var_values[i] = strdup(value);
            return;
        }
    }
    if (var_count < MAX_VARS) {
        var_names[var_count] = strdup(name);
        var_values[var_count] = strdup(value);
        var_count++;
    }
}

char *get_var(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_names[i], name) == 0)
            return var_values[i];
    }
    return NULL;
}

void unset_var(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_names[i], name) == 0) {
            free(var_names[i]);
            free(var_values[i]);
            for (int j = i; j < var_count - 1; j++) {
                var_names[j] = var_names[j + 1];
                var_values[j] = var_values[j + 1];
            }
            var_count--;
            return;
        }
    }
}

char* read_cmd(char* prompt, FILE* fp) {
    char* input = readline(prompt);

    if (input == NULL) {  // Ctrl+D
        return NULL;
    }

    if (strlen(input) > 0) {
        add_history(input);  // Adds to Readline's built-in history
    }

    return input;
}
/*
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }
    
    cmdline[pos] = '\0';
    return cmdline;
}*/

char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n')
        return NULL;

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* token = strtok(cmdline, " \t");
    int argnum = 0;

    while (token != NULL && argnum < MAXARGS) {
		if (token[0] == '$') {
   	 char *val = get_var(token + 1);
   	 if (val) token = val;
		}


        strncpy(arglist[argnum], token, ARGLEN - 1);
        argnum++;
        token = strtok(NULL, " \t");
    }

    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

int handle_builtin(char **arglist) {
    if (arglist == NULL || arglist[0] == NULL)
        return 0;

    // exit
    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    // cd
    else if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else if (chdir(arglist[1]) != 0) {
            perror("cd failed");
        }
        return 1;
    }
    // help
    else if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>   - Change directory\n");
        printf("  help       - Show this help message\n");
        printf("  jobs       - Show background jobs (not yet implemented)\n");
        printf("  exit       - Exit the shell\n");
        return 1;
    }

    // jobs
	else if (strcmp(arglist[0], "jobs") == 0) {
    printf("Active background jobs:\n");
    for (int i = 0; i < job_count; i++) {
        if (bg_jobs[i] > 0) {
            printf("[%d] PID %d\n", i + 1, bg_jobs[i]);
        }
    }}
	else if (strchr(arglist[0], '=') != NULL) {
    char *name = strtok(arglist[0], "=");
    char *value = strtok(NULL, "=");
    if (name && value)
        set_var(name, value);
    return 1;
}
else if (strcmp(arglist[0], "unset") == 0 && arglist[1]) {
    unset_var(arglist[1]);
    return 1;
}

  
    

    // Not a built-in
    return 0;
}
void add_to_history(const char *cmd) {
    if (cmd == NULL || strlen(cmd) == 0)
        return;

    // If full, free oldest command
    if (history_count == HISTORY_SIZE) {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
            history[i - 1] = history[i];
        history_count--;
    }

    history[history_count] = strdup(cmd);
    history_count++;
}

void show_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d  %s\n", i + 1, history[i]);
    }
}

char* get_history_command(int index) {
    if (index < 1 || index > history_count)
        return NULL;
    return history[index - 1];
}

#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;
while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
    // Handle !n command before adding to history
    if (cmdline[0] == '!' && strlen(cmdline) > 1) {
        int cmd_num = atoi(cmdline + 1);
        char *prev_cmd = get_history_command(cmd_num);
        if (prev_cmd == NULL) {
            printf("No such command in history.\n");
            free(cmdline);
            continue;
        } else {
            printf("%s\n", prev_cmd);
            free(cmdline);
            cmdline = strdup(prev_cmd);
        }
    }

    if ((arglist = tokenize(cmdline)) != NULL) {
        add_to_history(cmdline); // Add after confirming it's valid input

        if (!handle_builtin(arglist)) {
            execute(arglist);
        }

        for (int i = 0; arglist[i] != NULL; i++)
            free(arglist[i]);
        free(arglist);
    }

    free(cmdline);
}

}

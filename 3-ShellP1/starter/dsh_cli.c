#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dshlib.h"

int main() {
    char cmd_buff[SH_CMD_MAX];
    int rc = 0;
    command_list_t clist;

    while(1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        
        // Remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // Check for empty command
        if (strlen(cmd_buff) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        // Check for exit command
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            exit(0);
        }

        // Initialize clist
        memset(&clist, 0, sizeof(command_list_t));

        // Parse the command
        rc = build_cmd_list(cmd_buff, &clist);

        // Handle return codes
        if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        } else if (rc == OK) {
            printf(CMD_OK_HEADER, clist.num);
            // Print each command
            for (int i = 0; i < clist.num; i++) {
                printf("<%d> %s", i + 1, clist.commands[i].exe);
                if (strlen(clist.commands[i].args) > 0) {
                    printf(" [%s]", clist.commands[i].args);
                }
                printf("\n");
            }
        }
    }
    return 0;
}

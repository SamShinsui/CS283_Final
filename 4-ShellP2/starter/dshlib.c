#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

// Helper function to check if character is within quotes
static bool is_within_quotes(const char *str, int pos) {
    bool within_quotes = false;
    for (int i = 0; i < pos; i++) {
        if (str[i] == '"') within_quotes = !within_quotes;
    }
    return within_quotes;
}

// Helper function to trim whitespace
static char* trim(char* str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = 0;
    return str;
}

// Initialize cmd_buff structure
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = (char *)malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(char *) * CMD_ARGV_MAX);
    return OK;
}

// Free cmd_buff structure
int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}

// Clear cmd_buff for reuse
int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(char *) * CMD_ARGV_MAX);
    return OK;
}

// Parse command line into cmd_buff structure
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    clear_cmd_buff(cmd_buff);
    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';

    char *curr = cmd_buff->_cmd_buffer;
    bool in_quotes = false;
    bool in_token = false;
    char *token_start = NULL;

    // Skip leading spaces
    while (*curr && isspace(*curr)) curr++;
    if (!*curr) return WARN_NO_CMDS;

    token_start = curr;
    cmd_buff->argc = 0;

    while (*curr) {
        if (*curr == '"') {
            if (!in_token) {
                token_start = curr + 1;
                in_token = true;
            } else if (in_quotes) {
                *curr = '\0';
                in_token = false;
            }
            in_quotes = !in_quotes;
        } else if (isspace(*curr) && !in_quotes) {
            if (in_token) {
                *curr = '\0';
                cmd_buff->argv[cmd_buff->argc++] = token_start;
                in_token = false;
            }
        } else if (!in_token) {
            token_start = curr;
            in_token = true;
        }

        if (cmd_buff->argc >= CMD_ARGV_MAX - 1) {
            return ERR_TOO_MANY_COMMANDS;
        }
        curr++;
    }

    // Handle the last token if it exists
    if (in_token) {
        cmd_buff->argv[cmd_buff->argc++] = token_start;
    }

    // Null terminate the argv array
    cmd_buff->argv[cmd_buff->argc] = NULL;

    return OK;
}

// Match built-in commands
Built_In_Cmds match_command(const char *input) {
    if (!input) return BI_NOT_BI;
    if (strcmp(input, EXIT_CMD) == 0) return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0) return BI_CMD_CD;
    if (strcmp(input, "dragon") == 0) return BI_CMD_DRAGON;
    return BI_NOT_BI;
}

// Execute built-in commands
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds type = match_command(cmd->argv[0]);
    
    switch (type) {
        case BI_CMD_EXIT:
            return BI_CMD_EXIT;
            
        case BI_CMD_CD:
            if (cmd->argc > 1) {
                if (chdir(cmd->argv[1]) != 0) {
                    perror("cd");
                    return BI_RC;
                }
            }
            return BI_EXECUTED;
            
        case BI_CMD_DRAGON:
            printf("[DRAGON for extra credit would print here]\n");
            return BI_EXECUTED;
            
        default:
            return BI_NOT_BI;
    }
}

// Execute external command using fork/exec
int exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();
    int status;

    if (pid < 0) {
        perror("fork");
        return ERR_EXEC_CMD;
    }
    
    if (pid == 0) { // Child process
        execvp(cmd->argv[0], cmd->argv);
        // If execvp returns, there was an error
        perror("execvp");
        exit(1);
    } else { // Parent process
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

// Main command loop
int exec_local_cmd_loop() {
    cmd_buff_t cmd;
    char input_buffer[SH_CMD_MAX];
    int rc;

    // Initialize cmd_buff structure
    if (alloc_cmd_buff(&cmd) != OK) {
        printf("Failed to allocate command buffer\n");
        return ERR_MEMORY;
    }

    while (1) {
        printf("%s", SH_PROMPT);
        
        if (fgets(input_buffer, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remove trailing newline
        input_buffer[strcspn(input_buffer, "\n")] = '\0';

        // Build command buffer
        rc = build_cmd_buff(input_buffer, &cmd);
        
        if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }
        
        if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        }

        // Handle built-in commands
        Built_In_Cmds cmd_type = exec_built_in_cmd(&cmd);
        
        if (cmd_type == BI_CMD_EXIT) {
            free_cmd_buff(&cmd);
            return OK_EXIT;
        }
        
        if (cmd_type == BI_EXECUTED || cmd_type == BI_RC) {
            continue;
        }

        // Execute external command
        rc = exec_cmd(&cmd);
        if (rc != 0) {
            printf("Command failed with return code %d\n", rc);
        }
    }

    free_cmd_buff(&cmd);
    return OK;
}

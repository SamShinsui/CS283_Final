#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dshlib.h"

// Helper function to trim whitespace from both ends of a string
static char* trim(char* str) {
    char* end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) // All spaces?
        return str;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end + 1) = 0;
    
    return str;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *cmd_copy = strdup(cmd_line);
    char *trimmed_cmd = trim(cmd_copy);
    
    // Check for empty command
    if (strlen(trimmed_cmd) == 0) {
        free(cmd_copy);
        return WARN_NO_CMDS;
    }
    
    // Split by pipe character
    char *pipe_token = strtok(trimmed_cmd, PIPE_STRING);
    clist->num = 0;
    
    while (pipe_token != NULL && clist->num < CMD_MAX) {
        char *curr_cmd = trim(pipe_token);
        
        // Skip empty commands between pipes
        if (strlen(curr_cmd) == 0) {
            pipe_token = strtok(NULL, PIPE_STRING);
            continue;
        }
        
        // Get the executable (first token before space)
        char *space_pos = strchr(curr_cmd, SPACE_CHAR);
        
        if (space_pos != NULL) {
            // Have both executable and arguments
            int exe_len = space_pos - curr_cmd;
            if (exe_len >= EXE_MAX) {
                free(cmd_copy);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strncpy(clist->commands[clist->num].exe, curr_cmd, exe_len);
            clist->commands[clist->num].exe[exe_len] = '\0';
            
            // Copy arguments (trimmed)
            char *args = trim(space_pos + 1);
            if (strlen(args) >= ARG_MAX) {
                free(cmd_copy);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strcpy(clist->commands[clist->num].args, args);
        } else {
            // Only executable, no arguments
            if (strlen(curr_cmd) >= EXE_MAX) {
                free(cmd_copy);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strcpy(clist->commands[clist->num].exe, curr_cmd);
            clist->commands[clist->num].args[0] = '\0';
        }
        
        clist->num++;
        pipe_token = strtok(NULL, PIPE_STRING);
    }
    
    // Check if we have more commands (exceeded CMD_MAX)
    if (pipe_token != NULL) {
        free(cmd_copy);
        return ERR_TOO_MANY_COMMANDS;
    }
    
    free(cmd_copy);
    return OK;
}

#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

//INCLUDES for extra credit
//#include <signal.h>
//#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"

/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 */
int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);

    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.
 */
int boot_server(char *ifaces, int port){
    int svr_socket;
    int ret;
    int enable = 1;
    
    struct sockaddr_in addr;

    // Create the server socket
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Set socket options to reuse address
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    
    // Set up server address structure
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, ifaces, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Bind socket to address
    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    /*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     */
    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.
 */
int process_cli_requests(int svr_socket){
    int     cli_socket;
    int     rc = OK;    
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while(1){
        // Accept client connections
        cli_socket = accept(svr_socket, (struct sockaddr *)&client_addr, &client_len);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }
        
        printf("New client connected\n");
        
        // Process client requests
        rc = exec_client_requests(cli_socket);
        
        // Close client socket
        close(cli_socket);
        
        // If client requested server to stop, break the loop
        if (rc == OK_EXIT) {
            printf(RCMD_MSG_SVR_STOP_REQ);
            break;
        } else {
            printf(RCMD_MSG_CLIENT_EXITED);
        }
    }

    return rc;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *
 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 */
int send_message_eof(int cli_socket){
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len){
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 */
int send_message_string(int cli_socket, char *buff){
    int send_len = strlen(buff);
    int sent_len;
    
    sent_len = send(cli_socket, buff, send_len, 0);
    
    if (sent_len != send_len) {
        fprintf(stderr, CMD_ERR_RDSH_SEND, sent_len, send_len);
        return ERR_RDSH_COMMUNICATION;
    }
    
    return OK;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands.
 */
int exec_client_requests(int cli_socket) {
    int io_size;
    command_list_t cmd_list;
    int rc;
    int cmd_rc;
    int last_rc = 0;
    char *io_buff;
    Built_In_Cmds bi_result;

    // Allocate buffer for I/O
    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL){
        return ERR_RDSH_SERVER;
    }

    while(1) {
        // Receive command from client
        io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ - 1, 0);
        
        // Check for errors or connection closed
        if (io_size < 0) {
            perror("recv");
            free(io_buff);
            return ERR_RDSH_COMMUNICATION;
        }
        
        if (io_size == 0) {
            // Client closed connection
            free(io_buff);
            return OK;
        }
        
        // Ensure buffer is null-terminated
        io_buff[io_size] = '\0';
        
        printf("Received command: %s\n", io_buff);
        
        // Check for exit command directly (faster than parsing)
        if (strcmp(io_buff, EXIT_CMD) == 0) {
            printf("Client requested exit\n");
            send_message_string(cli_socket, "Goodbye!\n");
            send_message_eof(cli_socket);
            free(io_buff);
            return OK;
        }
        
        // Check for stop-server command directly
        if (strcmp(io_buff, "stop-server") == 0) {
            printf("Client requested server stop\n");
            send_message_string(cli_socket, "Server stopping...\n");
            send_message_eof(cli_socket);
            free(io_buff);
            return OK_EXIT;
        }
        
        // Parse the command
        memset(&cmd_list, 0, sizeof(command_list_t));
        rc = build_cmd_list(io_buff, &cmd_list);
        
        // Handle command parsing errors
        if (rc != OK) {
            if (rc == WARN_NO_CMDS) {
                send_message_string(cli_socket, CMD_WARN_NO_CMD);
            } else if (rc == ERR_TOO_MANY_COMMANDS) {
                char error_msg[100];
                sprintf(error_msg, CMD_ERR_PIPE_LIMIT, CMD_MAX);
                send_message_string(cli_socket, error_msg);
            } else {
                // Other error
                char error_msg[100];
                sprintf(error_msg, CMD_ERR_RDSH_ITRNL, rc);
                send_message_string(cli_socket, error_msg);
            }
            send_message_eof(cli_socket);
            continue;
        }
        
        // Check for built-in commands in the first command
        if (cmd_list.num > 0) {
            bi_result = exec_built_in_cmd(&cmd_list.commands[0]);
            
            if (bi_result == BI_CMD_EXIT) {
                // Client wants to exit
                send_message_string(cli_socket, "Goodbye!\n");
                send_message_eof(cli_socket);
                free_cmd_list(&cmd_list);
                free(io_buff);
                return OK;
            } else if (bi_result == BI_CMD_STOP_SVR) {
                // Client wants to stop server
                send_message_string(cli_socket, "Server stopping...\n");
                send_message_eof(cli_socket);
                free_cmd_list(&cmd_list);
                free(io_buff);
                return OK_EXIT;
            } else if (bi_result == BI_EXECUTED) {
                // Built-in command was executed
                send_message_string(cli_socket, "Command executed.\n");
                send_message_eof(cli_socket);
                free_cmd_list(&cmd_list);
                continue;
            }
        }
        
        // Execute the command pipeline
        printf(RCMD_MSG_SVR_EXEC_REQ, io_buff);
        cmd_rc = rsh_execute_pipeline(cli_socket, &cmd_list);
        
        // Store the return code
        last_rc = cmd_rc;
        
        // Send the EOF marker to indicate end of command output
        send_message_eof(cli_socket);
        
        // Free command list resources
        free_cmd_list(&cmd_list);
    }

    free(io_buff);
    return OK;
}

/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];  // Array of pipes
    pid_t pids[clist->num];
    int  pids_st[clist->num];         // Array to store process IDs
    int exit_code = 0;

    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_RDSH_CMD_EXEC;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        // Fork a child process
        pids[i] = fork();

        if (pids[i] < 0) {
            // Error forking
            perror("fork");
            
            // Close all pipes on error
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            return ERR_RDSH_CMD_EXEC;
        } 
        else if (pids[i] == 0) {
            // Child process
            
            // Set up input redirection
            if (i == 0) {
                // First command: stdin comes from client socket
                dup2(cli_sock, STDIN_FILENO);
            } else {
                // Not first command: stdin comes from previous pipe
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Set up output redirection
            if (i == clist->num - 1) {
                // Last command: stdout and stderr go to client socket
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            } else {
                // Not last command: stdout goes to next pipe, stderr to client
                dup2(pipes[i][1], STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            }
            
            // Close all pipe ends in child
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            
            // If execvp returns, there was an error
            perror(clist->commands[i].argv[0]);
            exit(ERR_RDSH_CMD_EXEC);
        }
    }

    // Parent process: close all pipe ends
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    // Get exit code of last process
    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);
    
    // Check for special return codes
    for (int i = 0; i < clist->num; i++) {
        if (WEXITSTATUS(pids_st[i]) == EXIT_SC) {
            exit_code = EXIT_SC;
        }
    }
    
    return exit_code;
}

/**************   OPTIONAL STUFF  ***************/

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype)
    {
    case BI_CMD_DRAGON:
        printf("[DRAGON for extra credit would print here]\n");
        return BI_EXECUTED;
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_RC:
        return BI_CMD_RC;
    case BI_CMD_CD:
        if (cmd->argc > 1) {
            if (chdir(cmd->argv[1]) != 0) {
                perror("cd");
                return BI_NOT_BI;
            }
        }
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}

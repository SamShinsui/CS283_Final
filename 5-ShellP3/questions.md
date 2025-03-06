1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My shell forks multiple child processes when executing piped commands. To ensure all child processes complete before the shell continues accepting user input, I call waitpid() on all child processes in the parent process. If I forgot to call waitpid() on all child processes, they would become "zombie" processes, causing resource leaks. Although my pipe implementation isn't working correctly yet (grep shows usage message instead of filtering ls output), the waitpid() logic is properly implemented

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

The dup2() function is used to redirect input and output file descriptors. Closing unused pipe ends after calling dup2() is necessary because a pipe remains open as long as at least one file descriptor references it. My current implementation has issues with pipe handling which I'm trying to debug - likely related to improper file descriptor management or closing pipe ends incorrectly, causing grep to not receive the input from ls

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The cd command is implemented as a built-in rather than an external command because it needs to change the current working directory of the shell process itself. If cd were implemented as an external process, it would change its own working directory (as a child process), but when it terminates, the shell's working directory would remain unchanged. This part of my implementation is correctly handling built-in commands.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To modify my implementation to allow an arbitrary number of piped commands, I would replace the static arrays with dynamically allocated memory, implement a resizable command list structure, and add proper cleanup code. Once I fix the current pipe implementation issues, adding this extension would be a logical next step to improve the shell's functionality and flexibility.

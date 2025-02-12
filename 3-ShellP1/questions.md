1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?
> **Answer**: `fgets()` is a great choice for our shell program because it handles input in a way that's perfect for command lines. It reads until it finds a newline character or reaches the buffer size limit, which helps prevent buffer overflows. It's also really good at handling EOF (like when we press Ctrl+D), making our shell work well with both interactive use and script testing. Plus, it keeps all the spaces and special characters intact, which we need for parsing commands correctly.

2. You needed to use `malloc()` to allocate memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?
> **Answer**: Actually, we didn't need to use `malloc()` for cmd_buff in our implementation. We used a fixed-size array instead because the shell has a defined maximum command length (SH_CMD_MAX). Using a fixed array is simpler and safer here since we know the maximum size we need, and the buffer stays around for the whole program. The only malloc we used was in build_cmd_list() for string processing, but cmd_buff itself works fine as a regular array.

3. In `dshlib.c`, the function `build_cmd_list()` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?
> **Answer**: Trimming spaces is super important because it helps our shell process commands correctly. Think about typing "  ls  " - without trimming, our shell might look for a command literally named "  ls  " (with spaces) instead of just "ls". This would break command execution. Also, extra spaces around pipes could create empty commands, and spaces at the end of arguments could mess up how programs interpret their input. Users expect commands to work the same whether they accidentally add extra spaces or not, so trimming helps make our shell behave more like standard shells.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.
> **Answer**: Here are three key redirections we should add to our shell:
> 1. `command > output.txt` - Basic output redirection
>    - We'd need to handle creating/overwriting files
>    - Check if we have permission to write
> 
> 2. `command < input.txt` - Input redirection
>    - Need to check if the file exists
>    - Handle read permissions and errors
>
> 3. `command 2> errors.log` - Error redirection
>    - Have to manage separate output streams
>    - Make sure errors get written correctly
>
> The tricky parts would be managing file descriptors and handling errors properly. We'd also need to think about how to handle cases like when files don't exist or when we don't have permissions.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.
> **Answer**: While both handle I/O, pipes and redirection work differently. Pipes (like in `ls | grep txt`) connect two commands directly - the output of ls goes straight to grep. Redirection (like `ls > files.txt`) works with files instead. Pipes are temporary and only exist while the commands run, but redirection creates or modifies files that stick around. It's like pipes are for programs to talk to each other, while redirection is for saving output or reading input from files.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?
> **Answer**: Keeping STDERR separate from STDOUT is really important for a few key reasons. When we're writing scripts, we want to be able to save the normal output but still see any errors that happen. It also helps when we're piping commands - we don't want error messages getting mixed into our data stream and messing things up. Plus, it lets us handle errors differently, like saving them to a separate log file while showing the regular output on screen.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?
> **Answer**: For our shell, I think we should handle errors similar to bash. By default, we should show errors on the screen even if normal output is redirected somewhere else. We should definitely support the `2>&1` syntax to combine the streams when needed - this is super useful for logging everything together. We also need to make sure we pass along exit codes properly so scripts can check if commands worked. The main challenge would be managing all the file descriptors correctly and making sure we clean them up properly.

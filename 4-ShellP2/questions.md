1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: We need to use fork() before execvp() because execvp() basically takes over the entire process it's running in. Think of it like this - if we just called execvp() directly in our shell, it would replace our shell with the new command.By using fork() first, we create a copy of our shell (the child process) that can safely be replaced by execvp() while our original shell (the parent) keeps running.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  In my implementation, when fork() fails, it returns -1. When this happens, I handle it by checking for that negative return value and then:

--> Print an error message with perror("fork")
--> Return an error code
--> Let the shell keep running
This way if something goes wrong with fork(), the user gets an error message but the shell doesn't crash. I learned this is important because fork() can fail if the system is running out of resources, and we don't want our shell to crash when that happens.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp() uses the PATH environment variable to find commands we type. when I type something like 'ls', execvp() looks through all the directories listed in PATH to find the actual program. So if I type 'ls', it might find it in '/bin/ls'. That's why we can just type 'ls' instead of having to type '/bin/ls' every time. The 'p' in execvp() actually stands for PATH - it basically means "look in PATH to find this command." If the command isn't found in any of the PATH directories, that's when we get the "command not found" error.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The wait() call is super important in our shell! Without it, the parent process (our shell) would just keep going without waiting for the command to finish. That would be really messy because We'd get our shell prompt back before the command finished. Furthermore, The output would get all mixed up. 

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: WEXITSTATUS() gets the exit code from a command that finished running. When a program exits, it returns a number (usually 0-255) that tells us if it worked (0) or if something went wrong (any other number.  

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  In my build_cmd_buff() function, I had to handle quotes carefully because sometimes we need to include spaces in a single argument. Like if you want to echo "Hello World" as one thing, not two separate words. Here's what my code does: Keeps track of whether we're inside quotes or not. Keeps spaces between quotes (instead of splitting on them).(But it does fails the BATS test) 

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  The biggest change from the last assignment was switching from the command_list_t structure to cmd_buff_t. I had to remove all the pipe stuf.) 

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**: From what I learned, signals are like urgent messages that Linux processes can send to each other. They're different from other ways processes can communicate because they're super simple - they don't carry any data, just a signal number that means something specific.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**: 1. SIGTERM (15): This is like asking a program nicely to close. It's what happens when you click the X on a window.
                   2. SIGKILL (9): This is the "emergency stop" button - it forces a program to close RIGHT NOW. It's like using Task Manager to "End Task" when a program is frozen.
                  3. SIGINT (2): This is what happens when you press Ctrl+C in the terminal. It's like saying "hey, stop what you're doing!" 
- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process gets SIGSTOP, it freezes immediately. programs can't ignore or handle this signal - it's one of the few that always works. This makes sense because:Sometimes you really need to be able to stop a program
                  -->If programs could ignore SIGSTOP, they could become unstoppable
                  -->It's like having a pause button that always works
                  ---> You can think of it like the pause button on a video game 


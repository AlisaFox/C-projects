# Tiny Shell

## What:
First project of the year: I made a shell stimulator. It is a minimalistic one - thus the name tiny. 
This assignment was mostly used to learn about forks.

## What's happening?

The tiny shell works like the following. We read a line of input, if the input is valid (longer than a single newline character),
we run the command. Otherwise, the tiny shell waits for the next input line.  
 
``` 
while (1) { 
  line = get_a_line();
  if length(line) > 1
      my_system(line); 
  }
  ```
  
 These are the two main functions: `get_a_line()` which is pretty self explanitory and `my_system(line)` which takes a line and tries to execute it.
 It is a function that spawns (i.e., creates) a child process and runs the command you passed as the argument to the call,
 assuming the command you requested to run is
 a valid one and is present in the machine. For example, if you specify `/bin/ls` it is going to execute the directory lister command.
 Due to this, if an error occurs, the whole program should not crash.
 
 We also have internal commands:
  - `chdir` or `cd` changes the current working directory of the process
  - `history` lists the last 100 commands that were executed in the tiny shell
  - `limit` sets the upper limit for the allowed resource usage
    - used RLIMIT_AS: the maximum size of the process's virtual memory (address space) in bytes
    
 There are also a bunch of restrictions:
- Cursor based movements are not supported
  - ex. after you typed in a command, you can't click in the middle of it and delete those middle characters
- we can only pipe two commands
  - ex. `ls | wc` works but `cd \ | ls | wc` wouldn't
  - instead of using an anonymous pipe we use a FIFO
    - the FIFO is passed as an arg to the shell when we initialize it
    
   The last part is signal handling. Normally a program shuts off when you hit CTRL-C. That feature is turned off. Instead we ask user
 confirmation of shut down. CTRL-Z is just ignored. 
 
 ## To-Do
 Deal with the restrictions! Also when I try to pass in a list of commands in a text file using `< commands.txt`, the shell doesn't
 terminate. So I've got to debug the problem, maybe it doesn't see the EOF?

 

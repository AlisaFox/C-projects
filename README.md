# Projects in C

Most of these are projects based on Operating Systems material. 

In order from top:

### 1. Readers & Writers
This is the classical readers & writers problem. We have somebody trying to read from a file, and somebody trying to write in it.
We do not want allow both readers and writers to do so at the same time. This is a problem solved via mutual exlusion in the critical section.

I made two solutions: one that gave priority to the readers, and another gave priority to the writers, but was a lot more fair.

### 2. Simple File System
I wrote the code a stimulate a file system. It was a simple one because only one level was allowed: there was the root directory and the files, no sub-directories. There are three main parts: first the disk representation, where we have the data blocks. Then we had to decide how said data blocks were to be allocated to represent the memory. Finally, I wrote all the methods to create / delete/ open/ read in / write in files. 

The next step would be to make it mountable via FUSE. Right now you can only interact with it directly, via launching the Makefile. 

### 3. Tiny Shell
This is a pseudo-shell, called tiny because it had very minimal functionality. This project mainly taught me how to use fork to spawn child processes, and to play with fault tolerability. 

Another different part is that this shell only allows two piped commands (in the form `ls | wc` ), and without using an unnamed pipe. Instead I learned how to implement a FIFO.

There are also a couple of unique coomands, such as `limit` (to limit the amount of memory allowed), `history` (shows last 100 commands used), and `chdir` (unlike `ls` and `wc`, cannot be done properly as a child process) 



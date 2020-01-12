# Simple File System

## What:
This is part of a school project, where we were supposed to create a simple file system. There is a lot of explaining to do in
this one!

To simplify the assignment, SFS introduced many limitations such as restricted filename lengths, no user concept, no protection among files, no support for concurrent access, etc
It was also one level: we had a root directory and the files, but no-subdirectories (folders).

## What's happening right now?

SFS was implemented on an emulated disk system, which is described in `disk_emu.c` and `disk_emu.h`.
There are two programs to test it: `sfs_test.c` and `sfs_test2.c`.
These can be run with the `makefile`.
In the makefile, there is already a way to test FUSE. Read more about that in the to-do below.

The main players here are `sfs_api.c` and `sfs_api.h`. All the main functionality is in here. 
I divided the data blocks provided by the emulated disk into a superblock, directory, i-Node table, data blocks for writing to, and a free bit map. 
All of these were initialized by arrays to make things simple.

When the sfs was to be run, I would also add a file descriptor table. Basically, each file is represented by an i-Node. The i-Nodes here are 
nowhere near as complicated as they are in reality, for example we only have
single indirect pointers, no double / triple ones. . The root-directory is pointed to by the superblock. 

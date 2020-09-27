## All Interfaces
This module named module3 will create a set of inerfaces including dev, sys, proc and ioctl which all work relatively.  
For isntallation, first you should make the module.
```
$ sudo make
```
Then the module will be maked.
To install module you should run:
```
$ sudo insmod "module_name"
```
Module 3 will perform the following actions:
* Create a dev interface named "dmod".
  * Dev interfaca can take inputs in three ways including stack, fifo and stack-fifo.
  * The content of dev interface can be read in two way; fifo and stack.
* Create a sys interface with two attributes "sfifo" and "sstack".  
  * The method of writing in and reading of dev interface determined by attributed of sys interface. (by setting each of two attributes 1)
* Create 3 proc interfaces respectively named plog, pfifo, pstack
  * plog: Contains a report of dev interfaces activities.
  * pfifo: The content of fifo can be listed with this interface.
  * pstack: The content of stack can be listed with this interface.
* Define an ioctl interface to recet fifo, stack or both from userspace. This also included a simple C program named "userspace.c" to run these command from user-space.

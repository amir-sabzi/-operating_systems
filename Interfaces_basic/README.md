## Dev and Proc Interfaces
This directory contains two module named <b>module1</b> and <b>module2</b>.  
If you run the makefile by:
```
$ sudo make
```
Then both modules will be maked.
To install each module you should run
```
$ sudo insmod "module_name"
```
Module 1 performs the following actions:
* Create a dev interface named module1
* Create a proc interface named module1
* Will create a log in proc interface of all things written on the dev one.(The log includes time, date and number of bytes has been written on dev interface so far)  

Module 2 performs the following actions:
* Create two proc interfaces named proc1 and proc2 in a folder.
* If you echo each of these proc interfaces you should see contents of the other.


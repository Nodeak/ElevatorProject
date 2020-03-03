COP4610 Project 2

Kiara Boone
    Part 1:
    - Created empty program to test

    Part 2:
    - Created timer kernel module and implemented project

    Part 3:
    - Elevator movement and scheduling
    - Implemented proc output
    - Loading of Elevator
Kaedon Hamm
    Part 1:
    - Implement 5 syscalls within program for testing
    - Used strace to track all system calls from the program
        
    Part 2:
    - Inserted, checked proc, and removed the my_timer module
    
    Part3:
    - Set up structure of code (global vars, defines, functions)
    - Unloading of Elevator
    - Helped with proc output
    - Documentation & Testing

Contents of tar file:
README - Information about team and project files
Part 1:
◦ empty.c - empty C program
◦ part1.c - C program that issues exactly five system calls
◦ empty.trace - output of strace() when running empty.c
◦ part1.trace - output of strace() when running part1.c

Part 2:
◦ my_timer.c - create proc entry that will print current time as well as the amount
    of time that has passed since the last call (if valid)
◦ Makefile - Makefile for my_timer.c

Part 3:
◦ elevator.c - Kernel module for the elevator
◦ Makefile - Makefile for elevator.c

How to Compile:
Part 1:
Command line compilation
    > gcc -o empty.x empty.c
    > gcc -o part1.x part1.c

Part 2:
In directory with Makefile and my_timer.c
> make

Part 3:
In directory with elevator.c and Makefile
> sudo make

To insert module:
> sudo insmod elevator.ko

To remove module:
> make stop
> sudo rmmod elevator

Known Bugs and Unfinished Portions of the Project:
There are not locks on every instance that shared data is read/written to. When some locks were implemented, 
it caused the program to crash/ not run at all.

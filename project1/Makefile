# Name: Thinh Nguyen

# NetID: ngt218

# Description: This program implements a RUSH shell 
# - a basic Unix shell that executes commands, handles 
# I/O redirection, and supports parallel process execution. 
# The shell provides essential built-in commands and external 
# program execution through a configurable search path, 
# demonstrating fundamental Unix shell functionality.

rush: main.o shell.o path.o utils.o
	gcc -Wall main.o shell.o path.o utils.o -o rush

main.o: main.c
	gcc -Wall -c main.c

shell.o: shell.c
	gcc -Wall -c shell.c

path.o: path.c
	gcc -Wall -c path.c

utils.o: utils.c
	gcc -Wall -c utils.c

clean:
	rm -f *.o

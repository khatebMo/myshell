all : myshell

myshell :	myshell.h	myshell.c 
	gcc -o myshell myshell.c

clean :
	rm myshell

all: builtins.o dsh.o
	gcc -Wall -o dsh main.c builtins.o dsh.o

builtins.o: builtins.h builtins.c
	gcc -Wall -c builtins.c

dsh.o: dsh.h dsh.c
	gcc -Wall -c dsh.c

clean:
	rm -f dsh *.o

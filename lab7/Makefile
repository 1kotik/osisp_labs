CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -pthread

main.o: main.c
	gcc $(CFLAGS) -c main.c -o main.o

lab7: main.o
	gcc main.o -o lab7
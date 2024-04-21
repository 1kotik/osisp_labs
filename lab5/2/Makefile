CFLAGS=-W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -pthread

utilities.o: utilities.c
	gcc $(CFLAGS) -c utilities.c -o utilities.o

main.o: main.c
	gcc $(CFLAGS) -c main.c -o main.o

lab5_2: utilities.o main.o
	gcc utilities.o main.o -o lab5_2

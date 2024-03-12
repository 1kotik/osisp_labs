utilities.o: utilities.c
	gcc -c utilities.c -o utilities.o

parent.o: parent.c 
	gcc -c parent.c -o parent.o
parent: parent.o utilities.o
	gcc parent.o utilities.o -o parent

child.o: child.c 
	gcc -c child.c -o child.o
child: child.o utilities.o
	gcc child.o utilities.o -o child
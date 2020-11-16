CC=gcc
CFLAGS=-Wall -std=c99 -pthread

all: line_processor

line_processor: main.o helpers.o 
	gcc -g -Wall -std=c99 -pthread -o line_processor main.o helpers.o

helpers.o: helpers.c helpers.h
	gcc -g -Wall -std=c99 -pthread -c helpers.c

main.o: main.c 
	gcc -g -Wall -std=c99 -pthread -c main.c

clean:
	-rm *.o line_processor

memory_check_full:
	valgrind --leak-check=full --show-leak-kinds=all ./line_processor

memory_check:
	valgrind --leak-check=full ./line_processor 

zip: 
	zip -r slusherg_program4.zip helpers.c helpers.h makefile README.txt main.c

all: trace

trace: LC4.o loader.o trace.c
	#
	#NOTE: CIS 240 students - this Makefile is broken, you must fix it before it will work!!
	#
	clang -Wall -g LC4.o loader.o trace.c -o trace

LC4.o: LC4.h LC4.c
	clang -c LC4.c

loader.o: loader.h loader.c
	clang -c loader.c


clean:
	rm -rf *.o

clobber: clean
	rm -rf trace

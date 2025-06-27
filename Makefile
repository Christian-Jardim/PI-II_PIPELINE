all:
	gcc -c pipeline.c -lncurses
	gcc -c main.c -lncurses
	gcc -o exec pipeline.o main.o -lncurses

clean:
	rm -f *.o exec

run:
	./exec

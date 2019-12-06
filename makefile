all: tetris_final.o
	gcc tetris_final.c -lncurses

clean:
	rm a.out *.o

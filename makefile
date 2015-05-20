CFLAGS = -Wall -Werror -pedantic -std=c99

server: main.o game.o
	$(CC) -pthread main.o game.o -o server

main.o: main.c game.h
	$(CC) $(CFLAGS) main.c -c -o main.o

game.o: game.c game.h
	$(CC) $(CFLAGS) game.c -c -o game.o

clean:
	rm -f server game.o main.o

.PHONY: clean

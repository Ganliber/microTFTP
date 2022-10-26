
CC=gcc

client:client.o
	$(CC) -o client client.c wrap.c wrap.h data.h

.PHONY:clean

clean:
	rm client client.o
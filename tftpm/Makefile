
CC=gcc

client:client.o
	$(CC) -o client client.c client.h wrap.c wrap.h data.h

.PHONY:clean

clean:
	rm client client.o

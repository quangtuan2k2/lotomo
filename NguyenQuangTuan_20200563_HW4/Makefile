CC = gcc

all: client server

client: ./UDP_Client/client.c
		${CC} ./UDP_Client/client.c -o client

server: ./UDP_Server/server.c
		${CC} ./UDP_Server/server.c -o server

clean:
		rm -f *.o *~
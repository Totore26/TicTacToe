server: server.o funzioni.o
	gcc -pthread server.o funzioni.o -o server

server.o:
	gcc -c -pthread server.c -o server.o

funzioni.o:
	gcc -c -pthread funzioni/funzioni.c -o funzioni.o

clean:
	rm -f server server.o funzioni.o

run_server: server
	./server 8080 127.0.0.1 5

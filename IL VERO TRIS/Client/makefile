client: client.o funzioni.o
	gcc client.o funzioni.o -o client

client.o:
	gcc -c client.c -o client.o

funzioni.o:
	gcc -c funzioni.c -o funzioni.o

clean:
	rm -f client client.o funzioni.o

run_client: client
	./client 127.0.0.1 8080 20

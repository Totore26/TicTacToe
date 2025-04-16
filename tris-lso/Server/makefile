CC = gcc
CFLAGS = -Wall -Wextra -O3 -march=native -pipe
LDFLAGS = -pthread
TARGET = server
OBJS = server.o funzioni.o 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

server.o: server.c funzioni.h strutture.h Comunicazione.h
	$(CC) $(CFLAGS) -c server.c

funzioni.o: funzioni.c funzioni.h strutture.h Comunicazione.h
	$(CC) $(CFLAGS) -c funzioni.c

clean:
	rm -f $(OBJS) $(TARGET)
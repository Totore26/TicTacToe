CC = gcc
CFLAGS = -Wall -Wextra -O3 -march=native -pipe
LDFLAGS = -pthread
TARGET = client
OBJS = client.o funzioni.o 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

client.o: client.c funzioni.h strutture.h Comunicazione.h
	$(CC) $(CFLAGS) -c client.c

funzioni.o: funzioni.c funzioni.h strutture.h Comunicazione.h
	$(CC) $(CFLAGS) -c funzioni.c

clean:
	rm -f $(OBJS) $(TARGET)
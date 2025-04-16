# Usa una base con compilatore C e strumenti di rete
FROM gcc:latest

# Crea una directory di lavoro
WORKDIR /app

# Copia i file necessari
COPY server.c ./  
COPY strutture.h ./
COPY funzioni/funzioni.c ./funzioni.c
COPY funzioni/funzioni.h ./funzioni.h

# Compila il server
RUN gcc server.c funzioni.c -o server -pthread

# Espone la porta TCP usata dal server (modifica se usi una porta diversa)
EXPOSE 8080

# Comando per eseguire il server
CMD ["./server"]

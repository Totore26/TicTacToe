# Usa una base con compilatore C
FROM gcc:latest

# Crea una directory di lavoro
WORKDIR /app

# Copia i file necessari
COPY client.c ./
COPY funzioni.c ./
COPY funzioni.h ./
COPY strutture.h ./
COPY ../Comunicazione.h ./   # <-- questo file è fuori da Client, lo copiamo manualmente

# Compila il client
RUN gcc client.c funzioni.c -o client

# Comando per eseguire il client
CMD ["./client"]

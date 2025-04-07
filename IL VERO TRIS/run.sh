#!/bin/bash

# Percorso assoluto dove si trovano gli eseguibili
WORK_DIR="/Users/salvatoretortora/Desktop/IL VERO TRIS"

# Entra nella directory dove si trovano i file server.c e client.c
cd "$WORK_DIR"

# Compila i file server.c e client.c
gcc server.c -o server -pthread
gcc client.c -o client

# Avvia il server in un nuovo terminale
echo "Avvio il server in un nuovo terminale..."
osascript -e "tell application \"Terminal\" to do script \"cd '$WORK_DIR'; ./server\""

# Attendi un po' per assicurarsi che il server sia avviato
sleep 1

# Avvia i due client in due nuovi terminali
echo "Avvio i due client in nuovi terminali..."
osascript -e "tell application \"Terminal\" to do script \"cd '$WORK_DIR'; ./client\""
osascript -e "tell application \"Terminal\" to do script \"cd '$WORK_DIR'; ./client\""

echo "Server e client avviati!"
 
#!/bin/bash

# Percorso alla cartella del progetto
PROGETTO="$HOME/Desktop/TicTacToe/tris-lso"

# Nome dei servizi definiti in docker-compose
SERVER_NAME="server"
CLIENT_NAME="client"
PROJECT_NAME="tris-lso"  # Questo è il nome che docker-compose usa nei container

# Vai nella directory del progetto
cd "$PROGETTO" || {
  echo "❌ Impossibile trovare la cartella del progetto: $PROGETTO"
  exit 1
}

# Chiedi il numero di client
read -rp "Inserisci il numero di client da avviare: " N

# Controllo input
if ! [[ "$N" =~ ^[0-9]+$ ]]; then
  echo "❌ Inserire un numero intero valido."
  exit 1
fi

echo "🧹 Controllo e rimozione di eventuali container esistenti..."

# Ferma ed elimina eventuali container client esistenti
for ((i = 1; i <= 50; i++)); do
  CONTAINER="${PROJECT_NAME}-${CLIENT_NAME}-${i}"
  if docker ps -a --format '{{.Names}}' | grep -q "^$CONTAINER$"; then
    echo "⚠️  Rimozione container esistente: $CONTAINER"
    docker rm -f "$CONTAINER" >/dev/null
  fi
done

# Ferma ed elimina il container server se esiste
SERVER_CONTAINER="${PROJECT_NAME}-${SERVER_NAME}-1"
if docker ps -a --format '{{.Names}}' | grep -q "^$SERVER_CONTAINER$"; then
  echo "⚠️  Rimozione container esistente: $SERVER_CONTAINER"
  docker rm -f "$SERVER_CONTAINER" >/dev/null
fi

# Avvia il server e i client
echo "🚀 Avvio del server e di $N client..."
docker compose up -d --scale "$CLIENT_NAME"="$N"

# Attendi qualche secondo per permettere la creazione dei container
sleep 3

# Apri una finestra terminale per ogni client e fai attach
for ((i = 1; i <= N; i++)); do
  TERMINAL_CMD="docker attach ${PROJECT_NAME}-${CLIENT_NAME}-${i}"
  osascript <<EOF
tell application "Terminal"
  do script "$TERMINAL_CMD"
  activate
end tell
EOF
done

echo "✅ Tutto pronto! Ogni client è stato aperto in una nuova finestra Terminal."

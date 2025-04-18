#!/bin/bash

# Nome: tictactoe_mac.sh
# Descrizione: Avvia il server e due client TicTacToe su macOS

# Imposta percorso di lavoro
PROJECT_DIR="$HOME/Desktop/TicTacToe/tris-lso"

# Pulisci eventuali build precedenti
cd "$PROJECT_DIR/Server" && rm -f server
cd "$PROJECT_DIR/Client" && rm -f client

# Compila server e client
echo "Compilazione server..."
cd "$PROJECT_DIR/Server" && gcc server.c funzioni.c -o server -pthread

echo "Compilazione client..."
cd "$PROJECT_DIR/Client" && gcc client.c funzioni.c -o client

# Avvia il server in una nuova finestra Terminal
osascript <<EOF
tell application "Terminal"
    do script "cd '$PROJECT_DIR/Server' && ./server; read -p 'Premi Invio per uscire...'"
end tell
EOF

# Attendi l'avvio del server
sleep 2

# Avvia il primo client
osascript <<EOF
tell application "Terminal"
    do script "cd '$PROJECT_DIR/Client' && ./client; read -p 'Premi Invio per uscire...'"
end tell
EOF

# Avvia il secondo client
osascript <<EOF
tell application "Terminal"
    do script "cd '$PROJECT_DIR/Client' && ./client; read -p 'Premi Invio per uscire...'"
end tell
EOF

osascript <<EOF
tell application "Terminal"
    do script "cd '$PROJECT_DIR/Client' && ./client; read -p 'Premi Invio per uscire...'"
end tell
EOF

echo "Server e client avviati!"
#!/bin/bash

# Exit on error
set -e

# Percorso alla directory del progetto
WORK_DIR="/home/merime/Desktop/Tris TOTOR/TicTacToe/IL VERO TRIS"

# Find suitable terminal emulator
if command -v gnome-terminal.real &> /dev/null; then
    TERMINAL="gnome-terminal.real"
    TERM_OPTS="-- bash -c"
elif command -v x-terminal-emulator &> /dev/null; then
    TERMINAL="x-terminal-emulator"
    TERM_OPTS="-e"
elif command -v xterm &> /dev/null; then
    TERMINAL="xterm"
    TERM_OPTS="-e"
elif command -v konsole &> /dev/null; then
    TERMINAL="konsole"
    TERM_OPTS="-e"
else
    echo "Error: No suitable terminal emulator found"
    exit 1
fi

# Entra nella directory del progetto
if ! cd "$WORK_DIR"; then
    echo "Error: Could not change to project directory"
    exit 1
fi

# Clean previous builds
rm -f Server/server Client/client

# Compila il server
echo "Compiling server..."
cd Server || exit 1
if ! gcc server.c funzioni/funzioni.c -o server -pthread; then
    echo "Error: Server compilation failed"
    exit 1
fi

# Compila il client 
echo "Compiling client..."
cd ../Client || exit 1
if ! gcc client.c funzioni.c -o client; then
    echo "Error: Client compilation failed"
    exit 1
fi

# Launch server and clients with proper terminal options
echo "Starting server..."
$TERMINAL $TERM_OPTS "cd '$WORK_DIR/Server' && ./server; read -p 'Press Enter to exit...'" &

# Wait for server startup
sleep 2

# Launch clients
echo "Starting clients..."
$TERMINAL $TERM_OPTS "cd '$WORK_DIR/Client' && ./client; read -p 'Press Enter to exit...'" &
$TERMINAL $TERM_OPTS "cd '$WORK_DIR/Client' && ./client; read -p 'Press Enter to exit...'" &

# Added wait to keep parent process running
echo "Server and clients started! Press Ctrl+C to exit"
wait
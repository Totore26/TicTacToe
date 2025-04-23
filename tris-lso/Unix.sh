#!/bin/bash

# Chiedi all'utente il numero di client da avviare
echo "Inserisci il numero di client da avviare:"
read n_client

# Verifica che l'input sia un numero intero maggiore o uguale a 1
if [ "$n_client" -lt 1 ]; then
    echo "Riprova, inserisci un numero valido di client"
    exit 1
fi

# Definisci i nomi dei servizi
PROJECT_NAME="tris-lso"
CLIENT_NAME="client"
SERVER_NAME="server"

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

# Avvia docker-compose con build e scaling per i client
if [[ "$OSTYPE" == "darwin" ]]; then
    osascript -e "tell application \"Terminal\" to do script \"docker-compose up --build --scale client=$n_client\""

elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if command -v gnome-terminal >/dev/null 2>&1; then
        gnome-terminal -- bash -c "docker-compose up --build --scale client=$n_client; exec bash"
    elif command -v konsole >/dev/null 2>&1; then
        konsole --hold -e "docker-compose up --build --scale client=$n_client"
    elif command -v xfce4-terminal >/dev/null 2>&1; then
        xfce4-terminal --hold -e "docker-compose up --build --scale client=$n_client"
    elif command -v xterm >/dev/null 2>&1; then
        xterm -hold -e "docker-compose up --build --scale client=$n_client"
    fi
fi

# Attendi che tutti i container attesi siano in esecuzione
all_containers_running="false"
while [ "$all_containers_running" = "false" ]; do
    all_containers_running="true"
    for (( i=1; i<=n_client; i++ )); do
        container_name="${PROJECT_NAME}-${CLIENT_NAME}-${i}"
        if ! docker ps --format "{{.Names}}" | grep -q "$container_name"; then
            all_containers_running="false"
            break
        fi
    done
    if [ "$all_containers_running" = "false" ]; then
        sleep 2
    fi
done

# Avvia una finestra terminale per ciascun client, collegandosi al rispettivo container
for (( i=1; i<=n_client; i++ )); do
    container_name="${PROJECT_NAME}-${CLIENT_NAME}-${i}"

    if [[ "$OSTYPE" == "darwin" ]]; then
        osascript -e "tell application \"Terminal\" to do script \"docker attach '$container_name'\""

    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v gnome-terminal >/dev/null 2>&1; then
            gnome-terminal -- bash -c "docker attach '$container_name'; exec bash"
        elif command -v konsole >/dev/null 2>&1; then
            konsole --hold -e "docker attach '$container_name'"
        elif command -v xfce4-terminal >/dev/null 2>&1; then
            xfce4-terminal --hold -e "docker attach '$container_name'"
        elif command -v xterm >/dev/null 2>&1; then
            xterm -hold -e "docker attach '$container_name'"
        fi
    fi
done

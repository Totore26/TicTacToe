#include "../strutture.h"
#include <stdio.h>
#include <stdlib.h>

extern Lobby lobby;


// ==========================================================
// FUNZIONI PER GESTIRE LA LOBBY
// ==========================================================

int emptyLobby() {
    pthread_mutex_lock(&lobby.lobbyMutex);
    for (int i = 0; i < MAX_GAMES; i++) {
        if (lobby.partita[i].statoPartita == PARTITA_IN_ATTESA) {
            return 0; // La lobby non è vuota
        }
    }
    pthread_mutex_unlock(&lobby.lobbyMutex);
    return 1; // La lobby è vuota
}

char *generaStringaPartiteDisponibili() {
    char *partiteDisponibili = malloc(BUFFER_SIZE);
    if (partiteDisponibili == NULL) {
        perror("[f generaStringaPartiteDisponibili] partiteDisponibili malloc failed");
        return NULL;
    }
    int index = 0;

    pthread_mutex_lock(&lobby.lobbyMutex);
    for (int i = 0; i < MAX_GAMES; i++) 
        if (lobby.partita[i].statoPartita == PARTITA_IN_ATTESA)
            index += snprintf(partiteDisponibili + index, "%d ", i); //uso snprintf per evitare partiteDisponibili overflow
    
    pthread_mutex_unlock(&lobby.lobbyMutex);

    if (index > 0) 
        partiteDisponibili[index - 1] = '\0'; // Rimuovo l'ultimo spazio
    else 
        partiteDisponibili[0] = '\0'; // setta a "" se non ci sono partite disponibili

    return partiteDisponibili;
}

// ==========================================================
// FUNZIONI PER GESTIRE LA PARTITA
// ==========================================================

void inizializzazioneGriglia( Partita *partita) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            partita->Griglia[i][j] = ' ';
        }
    }
}
int switchGiocatoreCorrente(int current) {
    return 1 - current; // alterna tra 0 e 1
}
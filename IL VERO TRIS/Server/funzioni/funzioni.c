#include "../strutture.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../Comunicazione.h"

extern Lobby lobby;


// ==========================================================
// FUNZIONI PER GESTIRE LA LOBBY
// ==========================================================

int emptyLobby() {

    pthread_mutex_lock(&lobby.lobbyMutex);
    for (int i = 0; i < MAX_GAMES; i++) 
        if (lobby.partita[i].statoPartita == PARTITA_IN_ATTESA) 
            return 0; // La lobby non è vuota

    pthread_mutex_unlock(&lobby.lobbyMutex);
    return 1; // La lobby è vuota
}

// genera stringhe del tipo "0 1 2 3\0"
char *generaStringaPartiteDisponibili() {
    char *partiteDisponibili = malloc(BUFFER_SIZE);
    if (partiteDisponibili == NULL) {
        perror("[f generaStringaPartiteDisponibili] partiteDisponibili malloc failed");
        return NULL;
    }
    int index = 0;
    index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "Partite disponibili:\n ");

    pthread_mutex_lock(&lobby.lobbyMutex);
    for (int i = 0; i < MAX_GAMES; i++) 
        if (lobby.partita[i].statoPartita == PARTITA_IN_ATTESA)
            index += snprintf(partiteDisponibili + index, "%d\n", i); //uso snprintf per evitare buffer overflow
    
    pthread_mutex_unlock(&lobby.lobbyMutex);

    if (index <= 0) 
        return MSG_NO_GAME; // se non ci sono partite disponibili

    return partiteDisponibili;
}

// gestione del riutilizzo id partite
int generazioneIdPartita(Lobby *lobby, const char *name, int host_socket) {
    if (emptyLobby())
        return 0;
    
    for (int i = 0; i < MAX_GAMES; i++) 
        if (lobby->partita[i].statoPartita == PARTITA_TERMINATA) 
            return i;

    return -1; // nessun id disponibile (errore)
}

int MaxPartiteRaggiunte() {
    int partiteAttive = 0;

    pthread_mutex_lock(&lobby.lobbyMutex);
    for (int i = 0; i < MAX_GAMES; i++) 
        if (lobby.partita[i].statoPartita != PARTITA_TERMINATA) 
            partiteAttive++;

    pthread_mutex_unlock(&lobby.lobbyMutex);
    if (partiteAttive >= MAX_GAMES)
        return 1; 
    return 0;
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
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
    index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "Partite disponibili:\n");

    pthread_mutex_lock(&lobby.lobbyMutex);
    for (int i = 0; i < MAX_GAMES; i++) 
        if (lobby.partita[i].statoPartita == PARTITA_IN_ATTESA)
            index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "%d\n", i); //uso snprintf per evitare partiteDisponibili overflow
    
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

// se le partite max torno 1, altrimenti torno 0
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

void inizializzaStatoPartite() {
    pthread_mutex_lock(&lobby.lobbyMutex);
    for (int i = 0; i < MAX_GAMES; i++) 
        lobby.partita[i].statoPartita = PARTITA_TERMINATA;
    pthread_mutex_unlock(&lobby.lobbyMutex);
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

// devo implementare elaboraMossa(partita->Griglia); ??????????????????????????????????????????????

void elaboraMossa(char *griglia, int mossa) {
    int row = (mossa - 1) / SIZE;
    int col = (mossa - 1) % SIZE;

    if (griglia[row * SIZE + col] == ' ') {
        griglia[row * SIZE + col] = 'X'; // o 'O' a seconda del giocatore
    } else {
        printf("Mossa non valida, riprova.\n");
    }
}



int switchGiocatoreCorrente(int current) {
    return 1 - current; // alterna tra 0 e 1
}

int check_winner(char symbol, Partita *partita) {
    // Check rows
    for(int i = 0; i < SIZE; i++) {
        if(partita->Griglia[i][0] == symbol && 
           partita->Griglia[i][1] == symbol && 
           partita->Griglia[i][2] == symbol)
            return 1;
    }
    
    // Check columns
    for(int i = 0; i < SIZE; i++) {
        if(partita->Griglia[0][i] == symbol && 
           partita->Griglia[1][i] == symbol && 
           partita->Griglia[2][i] == symbol)
            return 1;
    }
    
    // Check diagonals
    if(partita->Griglia[0][0] == symbol && 
       partita->Griglia[1][1] == symbol && 
       partita->Griglia[2][2] == symbol)
        return 1;
        
    if(partita->Griglia[0][2] == symbol && 
       partita->Griglia[1][1] == symbol && 
       partita->Griglia[2][0] == symbol)
        return 1;
        
    return 0;
}

int is_draw(Partita *partita) {
    for(int i = 0; i < SIZE; i++)
        for(int j = 0; j < SIZE; j++)
            if(partita->Griglia[i][j] == ' ')
                return 0;
    return 1;
}
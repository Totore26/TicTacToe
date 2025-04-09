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
        if (lobby.partita[i].statoPartita == PARTITA_IN_ATTESA) {
            pthread_mutex_unlock(&lobby.lobbyMutex);
            return 0; // La lobby non è vuota
        }
            
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
int generazioneIdPartita() {
    if (emptyLobby())
        return 0;
    
    for (int i = 0; i < MAX_GAMES; i++) 
        if (lobby.partita[i].statoPartita == PARTITA_TERMINATA) 
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

Partita *creaPartita(Giocatore *giocatore) {

    char buffer[BUFFER_SIZE];
    
    //controllo di non aver raggiunto il numero massimo di partite (se raggiunte torna 1)
    if (MaxPartiteRaggiunte()) {
        perror("[Lobby] Errore, numero massimo di partite raggiunto, informo il client\n");
        sprintf(buffer, MSG_SERVER_MAX_GAMES);
        if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
            perror("[Lobby] Errore nell'invio del messaggio di errore\n");
        }
        return NULL;
    }
    
    // creo la partita
    Partita *partita = malloc(sizeof(Partita));
    if (partita == NULL) {
        perror("[Lobby] Errore nell'allocazione della memoria per la partita\n");
        return NULL;
    }
    // inizializzo la partita
    partita->giocatoreAdmin = *giocatore;
    partita->statoPartita = PARTITA_IN_ATTESA;
    
    // aggiungo alla lobby la nuova partita
    int nuovoId = generazioneIdPartita();
    if (nuovoId == -1) {
        perror("[Lobby] Errore nella generazione dell'id della partita\n");
        partita->statoPartita = PARTITA_TERMINATA;
        return NULL;
    }

    pthread_mutex_lock(&lobby.lobbyMutex);
    lobby.partita[nuovoId] = *partita; 
    pthread_mutex_unlock(&lobby.lobbyMutex);

    // invio il messaggio di attesa al giocatore admin
    sprintf(buffer, MSG_WAITING_PLAYER);
    if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
        perror("[Lobby] Errore nell'invio del messaggio di attesa secondo giocatore\n");
        return NULL;
    }

    partita->statoPartita = PARTITA_IN_ATTESA; 

    return partita;
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

// questa torna una stringa (da provare)
char *grigliaFormattata(char griglia[SIZE][SIZE], int turno) {
    char *grigliaFormattata = malloc(BUFFER_SIZE);
    if (grigliaFormattata == NULL) {
        perror("[f grigliaFormattata] grigliaFormattata malloc failed");
        return NULL;
    }
    int index = 0;

    index += snprintf(grigliaFormattata + index, 1024 - index, "\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            index += snprintf(grigliaFormattata + index, 1024 - index, " %c ", griglia[i][j]);
            if (j < 2) index += snprintf(grigliaFormattata + index, 1024 - index, "|");
        }
        index += snprintf(grigliaFormattata + index, 1024 - index, "\n");
        if (i < 2) index += snprintf(grigliaFormattata + index, 1024 - index, "---|---|---\n");
    }
    if (turno % 2 == 0) {
        index += snprintf(grigliaFormattata + index, 1024 - index, "Sei il giocatore X\n");
    } else {
        index += snprintf(grigliaFormattata + index, 1024 - index, "Sei il giocatore O\n");
    }
    index += snprintf(grigliaFormattata + index, 1024 - index, "\n");

    return grigliaFormattata;
}

// devo implementare queste 2 ---->>>> ??????????????????????????????????????????????

int *convertiMossa(int mossa) {
    int *coordinate = malloc(2 * sizeof(int));
    if (coordinate == NULL) {
        perror("[f convertiMossa] coordinate malloc failed");
        return NULL;
    }
    coordinate[0] = (mossa - 1) / SIZE; // riga
    coordinate[1] = (mossa - 1) % SIZE; // colonna
    return coordinate;
}

// controlla se la mossa è valida
int is_valid_move(char board[3][3], int row, int col) {
    if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) 
        return 0; // mossa non valida

    if (board[row][col] != ' ') 
        return 0; // cella già occupata

    return 1; // mossa valida
}

int eseguiMossa(char board[3][3], int row, int col, char player) {
    if (is_valid_move(board, row, col)) {
        board[row][col] = player;
        return 1;
    }
    return 0;
}

int switchGiocatore(int current) {
    return !current;
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
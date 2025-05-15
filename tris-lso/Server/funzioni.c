#include "strutture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include "Comunicazione.h"

extern Lobby lobby;
extern Giocatori giocatori;


#define MAX_REGISTERED_PLAYERS MAX_CLIENTS
#define MAX_NAME_LENGTH 50

// Array globale per i nomi registrati
char nomiRegistrati[MAX_REGISTERED_PLAYERS][MAX_NAME_LENGTH];
int numeroNomiRegistrati = 0;

// ==========================================================
// FUNZIONI PER GESTIRE LA REGISTRAZIONE DEI NOMI
// ==========================================================

int nomeDuplicato(const char *nome) {
    for (int i = 0; i < numeroNomiRegistrati; i++) {
        if (strcmp(nomiRegistrati[i], nome) == 0) {
            return 1; // Nome duplicato
        }
    }
    return 0; // Nome non duplicato
}

void aggiungiNome(const char *nome) {
    if (numeroNomiRegistrati < MAX_REGISTERED_PLAYERS) {
        strncpy(nomiRegistrati[numeroNomiRegistrati], nome, MAX_NAME_LENGTH - 1);
        nomiRegistrati[numeroNomiRegistrati][MAX_NAME_LENGTH - 1] = '\0'; // Assicurati che sia terminato
        numeroNomiRegistrati++;
    }
}

void rimuoviNome(const char *nome) {
    for (int i = 0; i < numeroNomiRegistrati; i++) {
        if (strcmp(nomiRegistrati[i], nome) == 0) {
            // Sposta gli elementi successivi indietro di una posizione
            for (int j = i; j < numeroNomiRegistrati - 1; j++) {
                strncpy(nomiRegistrati[j], nomiRegistrati[j + 1], MAX_NAME_LENGTH);
            }
            numeroNomiRegistrati--;
            break;
        }
    }
}

// ==========================================================
// FUNZIONI PER LE NOTIFICHE
// ==========================================================


void notificaNuovaRegistrazione(Giocatori *giocatori, Giocatore *giocatore) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (giocatori->giocatore[i].socket != -1 &&
            giocatori->giocatore[i].stato == 2 && // nel menu principale
            giocatori->giocatore[i].socket != giocatore->socket) // non notificare se stesso
        {
            char notify[128];
            printf("[DEBUG] Invio notifica di nuova registrazione a %s (socket %d)\n", giocatori->giocatore[i].nome, giocatori->giocatore[i].socket);
            snprintf(notify, sizeof(notify), "%s:%s", MSG_NEW_USER_REGISTERED, giocatore->nome);
            send(giocatori->giocatore[i].socket, notify, strlen(notify), 0);
        }
    }
}

void notificaDisconnessione(Giocatori *giocatori, Giocatore *giocatore) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (giocatori->giocatore[i].socket != -1 &&
            giocatori->giocatore[i].stato == 2 && // nel menu principale
            giocatori->giocatore[i].socket != giocatore->socket) // non notificare se stesso
        {
            char notify[128];
            printf("[DEBUG] Invio notifica di disconnessione di %s (socket %d)\n", giocatore->nome, giocatori->giocatore[i].socket);
            snprintf(notify, sizeof(notify), "%s:%s", MSG_USER_DISCONNECTED, giocatore->nome);
            send(giocatori->giocatore[i].socket, notify, strlen(notify), 0);
        }
    }
}

void notificaNuovaPartita(Giocatori *giocatori, Partita *partita, int idPartita) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (giocatori->giocatore[i].socket != -1 &&
            giocatori->giocatore[i].stato == 2) // nel menu principale
        {
            char notify[256];
            printf("[DEBUG] Invio notifica di nuova partita a %s (socket %d)\n", giocatori->giocatore[i].nome, giocatori->giocatore[i].socket);

            // Assicurati che `nomePartita` e `giocatoreAdmin.nome` siano inizializzati
            if (partita->nomePartita[0] == '\0') {
                snprintf(partita->nomePartita, sizeof(partita->nomePartita), "Partita_%d", idPartita);
            }

            snprintf(notify, sizeof(notify), "%s:%s", 
                     MSG_NEW_GAME_CREATED, partita->giocatoreAdmin.nome);
            send(giocatori->giocatore[i].socket, notify, strlen(notify), 0);
        }
    }
}




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

const char *generaNomePartita(int id) {
    // Array di nomi predefiniti
    static const char *nomi[] = {
        "Vomero", "Acerra", "Marcianise", "Afragola", "Napoli",
        "Caivano", "Casalnuovo", "Pomigliano", "Ercolano", "Succivo",
        "Melito", "Marano", "Chiaiano", "Secondigliano", "Casoria",
        "Somma", "Frattamaggiore", "Volla", "Licola", "Pozzuoli",
        "Fuorigrotta", "Casapulla", "Aversa", "Caserta", "Pagani",
        "Parete", "Sant'Antimo", "Nola", "San Giuseppe", "San Vitaliano",
        "San Felice", "San Marco", "San Giorgio", "San Paolo", "San Pietro"
    };
    
    static int *nomi_usati = NULL;
    static int initialized = 0;
    
    // Inizializzazione una tantum
    if (!initialized) {
        srand(time(NULL));
        int numeroNomi = sizeof(nomi) / sizeof(nomi[0]);
        nomi_usati = calloc(numeroNomi, sizeof(int));
        if (!nomi_usati) {
            perror("Errore nell'allocazione memoria per nomi_usati");
            return nomi[0]; // fallback al primo nome
        }
        initialized = 1;
    }
    
    int numeroNomi = sizeof(nomi) / sizeof(nomi[0]);
    
    // Verifica se tutti i nomi sono stati usati
    int tutti_usati = 1;
    for (int i = 0; i < numeroNomi; i++) {
        if (!nomi_usati[i]) {
            tutti_usati = 0;
            break;
        }
    }
    
    // Se tutti i nomi sono stati usati, resetta l'array
    if (tutti_usati) {
        memset(nomi_usati, 0, numeroNomi * sizeof(int));
    }
    
    // Trova un nome non ancora usato
    int indice;
    do {
        indice = rand() % numeroNomi;
    } while (nomi_usati[indice]);
    
    nomi_usati[indice] = 1;
    return nomi[indice];
}

char *generaStringaPartiteDisponibili() {
    char *partiteDisponibili = malloc(BUFFER_SIZE);
    if (partiteDisponibili == NULL) {
        perror("[f generaStringaPartiteDisponibili] partiteDisponibili malloc failed");
        return NULL;
    }
    int index = 0;
    index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "╔══════════════════════════════════════════════════════════╗\n");
    index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "║       PARTITE DISPONIBILI                                ║\n");
    index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "╠══════════════════════════════════════════════════════════╣\n");
    index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "║  NOME PARTITA         │  ID PARTITA    │  ADMIN          ║\n");
    index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "╠══════════════════════════════════════════════════════════╣\n");

    pthread_mutex_lock(&lobby.lobbyMutex);
    for (int i = 0; i < MAX_GAMES; i++) {
        if (lobby.partita[i].statoPartita == PARTITA_IN_ATTESA) {
            // Controlla se il nome della partita è vuoto
            if (lobby.partita[i].nomePartita[0] == '\0') {
                const char *nomeGenerato = generaNomePartita(i);
                if (nomeGenerato != NULL) {
                    // Copia il nome generato nella struttura della partita
                    strncpy(lobby.partita[i].nomePartita, nomeGenerato, sizeof(lobby.partita[i].nomePartita) - 1);
                    lobby.partita[i].nomePartita[sizeof(lobby.partita[i].nomePartita) - 1] = '\0'; // Assicurati che sia terminata
                }
            }

            // Usa il nome della partita salvato nella struttura
            const char *nomePartita = lobby.partita[i].nomePartita;
            const char *nomeAdmin = lobby.partita[i].giocatoreAdmin.nome;
            index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "║  %-20s │  %-12d  │  %-14s ║\n", nomePartita, i, nomeAdmin);
        }
    }
    pthread_mutex_unlock(&lobby.lobbyMutex);

    if (index <= 0) {
        free(partiteDisponibili); // Libera la memoria se non ci sono partite disponibili
        return MSG_NO_GAME; // se non ci sono partite disponibili
    }
    index += snprintf(partiteDisponibili + index, BUFFER_SIZE - index, "╚══════════════════════════════════════════════════════════╝\n");
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
    pthread_mutex_init(&lobby.lobbyMutex, NULL);
    pthread_mutex_lock(&lobby.lobbyMutex);
    for (int i = 0; i < MAX_GAMES; i++) 
        lobby.partita[i].statoPartita = PARTITA_TERMINATA;
    pthread_mutex_unlock(&lobby.lobbyMutex);
}

void inizializzaGiocatori() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        giocatori.giocatore[i].socket = -1; // Inizializza il socket a -1 per indicare che non è connesso
        giocatori.giocatore[i].stato = 0; // Inizializza lo stato a 0 (giocatore non in lista partite)
        giocatori.giocatore[i].id = -1; // Inizializza l'id a -1 (non assegnato)
    }
}

int assegnazioneGiocatore(Giocatore giocatore) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (giocatori.giocatore[i].socket == -1) { // Trova un posto libero
            giocatori.giocatore[i] = giocatore;
            return i; // Restituisce l'indice del giocatore assegnato
        }
    }
    return -1; // Non ci sono posti liberi
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

    // Notifica a tutti i client nel menu principale della nuova partita
    notificaNuovaPartita(&giocatori, partita, nuovoId);
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
    index += snprintf(grigliaFormattata + index, 1024 - index, "╔══════════════════════════════╗\n");
    index += snprintf(grigliaFormattata + index, 1024 - index, "║        TAVOLO DI GIOCO       ║\n");
    index += snprintf(grigliaFormattata + index, 1024 - index, "╚══════════════════════════════╝\n\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            index += snprintf(grigliaFormattata + index, 1024 - index, " %c ", griglia[i][j]);
            if (j < 2) index += snprintf(grigliaFormattata + index, 1024 - index, "|");
        }
        index += snprintf(grigliaFormattata + index, 1024 - index, "\n");
        if (i < 2) index += snprintf(grigliaFormattata + index, 1024 - index, "---|---|---\n");
    }
    if (turno % 2 == 0) {
        index += snprintf(grigliaFormattata + index, 1024 - index, "\nSei il giocatore X\n");
    } else {
        index += snprintf(grigliaFormattata + index, 1024 - index, "\nSei il giocatore O\n");
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
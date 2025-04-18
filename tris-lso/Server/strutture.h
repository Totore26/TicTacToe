#ifndef STRUTTURE_H
#define STRUTTURE_H

#include <pthread.h>

#define MAX_GAMES       10
#define MAX_CLIENTS     (MAX_GAMES * 2)
#define SIZE            3                   // Dimensione della griglia
#define PORT            8080
#define BUFFER_SIZE     1024

// STATO PARTITA
#define PARTITA_IN_CORSO            0       // quando la partita è in corso
#define PARTITA_TERMINATA           1       // quando un giocatore vince o pareggia
#define PARTITA_IN_ATTESA           2       // quando si sceglie di partecipare a una partita

// la partita quanto termina rimane con stato in corso, passa allo stato terminata solo quando entrambi gli host hanno abbandonato

// ALTRO
#define CLEAR_SCREEN()  printf("\033[H\033[J")

// ==========================================================
// STRUTTURA DI UN GIOCATORE
// ==========================================================

typedef struct {
    int socket;     // socket associato al giocatore    // 'X' o 'O'
    int stato;      // stato del giocatore (1 se sta scegliendo una partita, 0 altrimenti)
    int id;         // id del giocatore assegnato nel vettore di giocatori
} Giocatore;


// ==========================================================
// STRUTTURA DI UNA PARTITA ATTIVA
// ==========================================================

typedef struct {
    char nomePartita[20];
    Giocatore giocatoreAdmin;
    Giocatore giocatoreGuest;
    int Vincitore;                      // la socket del vincitore   
    char Griglia[3][3];
    int statoPartita;                   // in corso, terminata, in attesa, nuova creazione
} Partita;

// ==========================================================
// STRUTTURA DI UNA LOBBY CHE TRACCIA LE PARTITE DISPONIBILI
// ==========================================================

typedef struct {
    Partita partita[MAX_GAMES];     // per tenere traccia delle partite attive
    pthread_mutex_t lobbyMutex;
} Lobby;

// ==========================================================
// STRUTTURA DI GIOCATORI CHE TRACCIA LE CONNESSIONI
// ==========================================================

typedef struct {
    Giocatore giocatore[MAX_CLIENTS];   // per tenere traccia dei giocatori connessi
} Giocatori;



//crea partita, unisciti, stampa ogni volta che qualcuno accede al server con un segnale per tutti i thread


#endif // STRUTTURE_H

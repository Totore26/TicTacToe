#include "strutture.h"
#define MAX_REGISTERED_PLAYERS MAX_CLIENTS
#define MAX_NAME_LENGTH 50

extern Lobby lobby;
extern char nomiRegistrati[MAX_REGISTERED_PLAYERS][MAX_NAME_LENGTH];
extern int numeroNomiRegistrati;

int nomeDuplicato(const char *nome);
void aggiungiNome(const char *nome);
void rimuoviNome(const char *nome);
void notificaNuovaRegistrazione(Giocatori *giocatori, Giocatore *giocatore);
void notificaDisconnessione(Giocatori *giocatori, Giocatore *giocatore);
int emptyLobby();
const char *generaNomePartita(int id);
char *generaStringaPartiteDisponibili();
void inizializzazioneGriglia(Partita *partita);
int switchGiocatore(int current);
int MaxPartiteRaggiunte();
int generazioneIdPartita();
void inizializzaStatoPartite();
void inizializzaGiocatori();
int assegnazioneGiocatore(Giocatore giocatore);
int eseguiMossa(char board[3][3], int row, int col, char player);
int is_valid_move(char board[3][3], int row, int col);
int check_winner(char symbol, Partita *partita);
int is_draw(Partita *partita);
char *grigliaFormattata(char griglia[SIZE][SIZE], int turno);
int *convertiMossa(int mossa);
Partita *creaPartita(Giocatore *giocatore);

#include "../strutture.h"
extern Lobby lobby;

int emptyLobby();
char *generaStringaPartiteDisponibili();
void inizializzazioneGriglia(Partita *partita);
int switchGiocatore(int current);
int MaxPartiteRaggiunte();
int generazioneIdPartita();
void inizializzaStatoPartite();
int eseguiMossa(char board[3][3], int row, int col, char player);
int is_valid_move(char board[3][3], int row, int col);
int check_winner(char symbol, Partita *partita);
int is_draw(Partita *partita);
char *grigliaFormattata(char griglia[SIZE][SIZE], int turno);
int *convertiMossa(int mossa);

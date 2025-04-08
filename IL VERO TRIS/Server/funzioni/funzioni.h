#include "../strutture.h"
extern Lobby lobby;

int emptyLobby();
char *generaStringaPartiteDisponibili();
void inizializzazioneGriglia(Partita *partita);
int switchGiocatore(int current);
int MaxPartiteRaggiunte();
int generazioneIdPartita();
void inizializzaStatoPartite();
void elaboraMossa(char *griglia, int mossa);
int check_winner(char symbol, Partita *partita);
int is_draw(Partita *partita);
char *grigliaFormattata(char griglia[SIZE][SIZE]);
int *convertiMossa(int mossa);

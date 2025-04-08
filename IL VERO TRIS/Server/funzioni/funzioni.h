#include "../strutture.h"
extern Lobby lobby;

int emptyLobby();
char *generaStringaPartiteDisponibili();
void inizializzazioneGriglia(Partita *partita);
int switchGiocatoreCorrente(int current);
int MaxPartiteRaggiunte();
int generazioneIdPartita();
int check_winner(char symbol, Partita *partita);
int is_draw(Partita *partita);

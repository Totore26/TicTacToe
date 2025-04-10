#ifndef FUNZIONI_H
#define FUNZIONI_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdbool.h>
#include "strutture.h"

#define CLEAR_SCREEN() printf("\033[H\033[J")

//si connette alla socket del server, restituisce il sd del client
void inizializza_socket();

//funzione che rappresenta il ciclo di vita del giocatore
void funzione_lobby();
void funzione_menu();
void funzione_crea_partita();
void funzione_entra_partita();
int get_valid_move(char *input);
int get_valid_match(char *input);
void play_again_menu();

//funzione che gestisce la partita tra 2 giocatori, incluse eventuali rivincite
void gioca_partita(const enum tipo_giocatore tipo_giocatore);

//aggiorna la griglia di gioco e il numero giocate, invia la giocata e l'esito della partita al server, restituisce l'esito
char invia_giocata(unsigned short int *n_giocate);
//riceve la giocata dal server, aggiorna la griglia, restituisce l'esito
char ricevi_giocata(unsigned short int *n_giocate);

//controlla chi ha vinto e restituisce l'esito
char controllo_esito(const unsigned short int *n_giocate);
//controlla se il giocatore ha inserito un input valido
bool controllo_giocata(const int giocata);

//inserisce nella board in base alla giocata del giocatore
void inserisci(const unsigned short int giocata, const unsigned short int n_giocate);
//stampa la griglia attuale
void stampa_griglia();

//gestiscono la richiesta di rivincita, restituendo true se la rivincita è stata accettata
bool rivincita_proprietario();
bool rivincita_avversario();

//manda un messaggio di errore e chiude il processo
void  gestisci_errore();


#endif
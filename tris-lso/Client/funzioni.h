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

//Funzione che gestisce la connessione al server
// e l'inizializzazione del socket
void inizializza_socket();

//Funzione che gestisce la chiusura del socket
// e la chiusura del programma
// Questa funzione viene chiamata quando il programma riceve un segnale SIGTERM
void SIGTERM_handler();

//Funzione che aspetta un input da parte dell'utente per continuare l'esecuzione
void attendo_invio();

//Funzione che gestisce il menu principale del client
void funzione_menu();

//Funzione che gestisce la creazione di una partita
void funzione_crea_partita();

//Funzione che gestisce l'entrata in una partita
void funzione_entra_partita();

//Funzione che ottiene un input valido da parte dell'utente per la selezione delle partite
int get_valid_match(char *input);

//Funzione che gestisce una partita giocata
void gioca_partita(const enum tipo_giocatore tipo_giocatore);

//Funzione che ottiene un input valido (1-9) da parte dell'utente per le mosse della partita
int get_valid_move(char *input);

//Funzione che gestisce il menu di fine partita in caso di vittoria
void play_again_menu();

//Funzione che gestisce il menu di fine partita in caso di pareggio
void play_again_menu_draw();

#endif
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
void attendo_invio();
void funzione_lobby();
void funzione_menu();
void funzione_crea_partita();
void funzione_entra_partita();
int get_valid_move(char *input);
int get_valid_match(char *input);
void play_again_menu();
void gioca_partita(const enum tipo_giocatore tipo_giocatore);
void play_again_menu_draw();



#endif
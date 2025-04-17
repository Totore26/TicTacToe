#ifndef STRUTTUREDATI_H
#define STRUTTUREDATI_H
#include "Comunicazione.h"

//dimensioni buffer per leggere e scrivere sulla socket
#define MAXLETTORE 1024
#define MAXSCRITTORE 16
#define BUFFER_SIZE 1024 

//serve alla funzione partita per gestire gli input
enum tipo_giocatore
{
    PROPRIETARIO,
    AVVERSARIO
};

extern int sd;

#endif
#ifndef PROTOCOLLO_H
#define PROTOCOLLO_H

/*
 * PROTOCOLLO DI COMUNICAZIONE CLIENT ↔ SERVER
 *
 * Tutti i messaggi sono stringhe ASCII terminate da '\n'.
 * I messaggi possono essere inviati/ricevuti tramite socket TCP.
 *
 * Notazione:
 *   C -> S : messaggio inviato dal CLIENT al SERVER
 *   S -> C : messaggio inviato dal SERVER al CLIENT
 *
 * Tutti i messaggi sono UPPERCASE per chiarezza.
 */

// ========== MESSAGGI INVIATI DAL CLIENT ==========

#define MSG_CLIENT_MOVE             "MOVE"  // seguito da due interi per la mossa

#define MSG_CLIENT_QUIT             "QUIT" // quando il client abbandona la partita dalla lobby

#define MSG_CLIENT_JOIN             "JOIN" // il client si unisce a una partita esistente

#define MSG_CLIENT_CREAATE          "CREATE" // il client crea una nuova partita

#define MSG_SERVER_START            "START" // entrambi i giocatori sono pronti, inizia la partita


// ========== MESSAGGI INVIATI DAL SERVER ==========

#define MSG_SERVER_MENU             "MENU"  // per stampare il menu di benvenuto

#define MSG_SERVER_WELCOME          "WELCOME"  // seguito da X o O per il messaggio all entrata in game

#define MSG_WAITING_PLAYER          "WAITING_PLAYER" // per segnalare che la partita è in attesa di un avversario

#define MSG_NO_GAME                 "NO_GAME" // per segnalare che non ci sono partite disponibili

#define MSG_JOIN_ERROR              "JOIN_ERROR" // errore durante l'unione a una partita

#define MSG_YOUR_TURN               "YOUR_TURN" // per segnalare che è il turno del giocatore

#define MSG_SERVER_OPPONENT_TURN    "OPPONENT_TURN" // per segnalare che è il turno dell avversario

#define MSG_CHOISE                  "CHOISE" // per segnalare che il giocatore può scegliere tra crea o unisciti

#define MSG_SERVER_BOARD            "BOARD"  // seguito da 9 caratteri

#define MSG_SERVER_WIN              "WIN"

#define MSG_SERVER_LOSE             "LOSE"

#define MSG_SERVER_DRAW             "DRAW"

#define MSG_SERVER_OPPONENT_LEFT    "OPPONENT_LEFT"

#endif // PROTOCOLLO_H


/* 
menu: 
1. Crea partita
2. Unisciti a partita
3. Esci




*/
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


/*
 * C -> S : "MOVE r c\n"
 * Il client invia una mossa alle coordinate (riga, colonna), 0-based.
 */
#define MSG_CLIENT_MOVE         "MOVE"  // seguito da due interi

/*
 * C -> S : "QUIT\n"
 * Il client abbandona la partita.
 */
#define MSG_CLIENT_QUIT         "QUIT"

/*
 * C -> S : "JOIN\n"
 * Il client si unisce alla partita.
 */
#define MSG_CLIENT_JOIN         "JOIN"

/*
 * C -> S : "QUIT\n"
 * richiesta di creazione partita
 */
#define MSG_CLIENT_QUIT         "CREATE"



// ========== MESSAGGI INVIATI DAL SERVER ==========

/*
 * S -> C : "WELCOME X\n" oppure "WELCOME O\n"
 * Il server assegna al client il simbolo di gioco.
 */
#define MSG_SERVER_WELCOME      "WELCOME"  // seguito da X o O

/*
 * S -> C : "START\n"
 * Entrambi i client sono connessi, la partita può iniziare.
 */
#define MSG_SERVER_START        "START"

/*
 * S -> C : "YOUR_TURN\n"
 * Il server indica che è il turno del client.
 */
#define MSG_SERVER_YOUR_TURN    "YOUR_TURN"

/*
 * S -> C : "OPPONENT_TURN\n"
 * È il turno dell’altro giocatore.
 */
#define MSG_SERVER_OPPONENT_TURN "OPPONENT_TURN"

/*
 * S -> C : "BOARD <stringa>\n"
 * Invia lo stato attuale della griglia. 9 caratteri: X, O, o -
 * Esempio: BOARD XOX-O-X--
 */
#define MSG_SERVER_BOARD        "BOARD"  // seguito da 9 caratteri

/*
 * S -> C : "INVALID_MOVE\n"
 * La mossa inviata era illegale (es. cella già occupata).
 */
#define MSG_SERVER_INVALID_MOVE "INVALID_MOVE"

/*
 * S -> C : "WIN\n"
 * Il client ha vinto.
 */
#define MSG_SERVER_WIN          "WIN"

/*
 * S -> C : "LOSE\n"
 * Il client ha perso.
 */
#define MSG_SERVER_LOSE         "LOSE"

/*
 * S -> C : "DRAW\n"
 * La partita è terminata in pareggio.
 */
#define MSG_SERVER_DRAW         "DRAW"

/*
 * S -> C : "OPPONENT_LEFT\n"
 * L’avversario si è disconnesso o ha abbandonato.
 */
#define MSG_SERVER_OPPONENT_LEFT "OPPONENT_LEFT"

#endif // PROTOCOLLO_H

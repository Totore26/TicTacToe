#ifndef PROTOCOLLO_H
#define PROTOCOLLO_H

/*
 * PROTOCOLLO DI COMUNICAZIONE CLIENT ↔ SERVER
 *
 * Tutti i messaggi sono UPPERCASE per chiarezza.
 * 
 */

// ========== MESSAGGI INVIATI DAL CLIENT ==========

#define MSG_CLIENT_MOVE             "MOVE"  // seguito da due interi per la mossa

#define MSG_CLIENT_QUIT             "QUIT" // quando il client abbandona la partita dalla lobby

#define MSG_CLIENT_JOIN             "JOIN" // il client si unisce a una partita esistente

#define MSG_CLIENT_ACCEPT           "ACCEPT" // il client accetta di unirsi a una partita

#define MSG_CLIENT_REFUSE           "REFUSE" // il client rifiuta di unirsi a una partita

#define MSG_CLIENT_CREAATE          "CREATE" // il client crea una nuova partita

#define MSG_CLIENT_REMATCH          "REMATCH" // il client richiede un rematch


// ========== MESSAGGI INVIATI DAL SERVER ==========

#define MSG_SERVER_MENU             "MENU"  // per stampare il menu di benvenuto (unisciti, partecipa, esci)

#define MSG_SERVER_START            "START" // entrambi i giocatori sono pronti, inizia la partita

#define MSG_SERVER_WELCOME          "WELCOME"  // seguito da X o O per il messaggio all entrata in game

#define MSG_WAITING_PLAYER          "WAITING_PLAYER" // per segnalare che la partita è in attesa di un avversario

#define MSG_WAITING_REMATCH         "WAITING_REMATCH" // quando l avversario deve aspettare la scelta del proprietario

#define MSG_NO_GAME                 "NO_GAME" // per segnalare che non ci sono partite disponibili

#define MSG_SERVER_MAX_GAMES        "MAX_GAMES" // per segnalare che sono state raggiunte le partite massime

#define MSG_JOIN_ERROR              "JOIN_ERROR" // errore durante l'unione a una partita

#define MSG_YOUR_TURN               "YOUR_TURN" // per segnalare che è il turno del giocatore

#define MSG_OPPONENT_TURN           "OPPONENT_TURN" // per segnalare che è il turno dell avversario

#define MSG_INVALID_MOVE            "INVALID_MOVE" // per segnalare che la mossa non è valida

#define MSG_SERVER_BOARD            "BOARD"  // seguito da 9 caratteri

#define MSG_SERVER_WIN              "WIN"   // per segnalare che il giocatore ha vinto

#define MSG_SERVER_LOSE             "LOSE"  // per segnalare che il giocatore ha perso

#define MSG_SERVER_DRAW             "DRAW"  // per segnalare che la partita è finita in pareggio

#define MSG_SERVER_OPPONENT_LEFT    "OPPONENT_LEFT" // per segnalare che l'avversario ha abbandonato la partita

#define MSG_SERVER_ADMIN_QUIT       "ADMIN_QUIT" // quando il proprietario decide di abbandonare la partita

#define MSG_SERVER_REMATCH          "REMATCHH" // quando il proprietario decide di fare un rematch (attendo una risposta)

#define MSG_SERVER_JOIN_REQUEST    "JOIN_REQUEST" // quando il proprietario decide di iniziare una (attendo una risposta)

#define MSG_SERVER_REFUSE           "REFUSE" // quando il proprietario decide di rifiutare l'avversario


#endif // PROTOCOLLO_H
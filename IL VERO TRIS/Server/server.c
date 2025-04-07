#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "strutture.h"
#include "../Comunicazione.h"

Lobby lobby;

// Thread per gestire una partita
void *threadPartita(void *arg) {
    Game *game = (Game *)arg;
    int players[2] = {game->player1, game->player2};
    char symbols[2] = {'X', 'O'};
    char buffer[1024];
    int current_player = 0;

    init_board();

    while (1) {
        sprintf(buffer, "Tocca a te, inserisci riga e colonna (0-2): ");
        send(players[current_player], buffer, strlen(buffer), 0);

        recv(players[current_player], buffer, sizeof(buffer), 0);
        int row = buffer[0] - '0', col = buffer[1] - '0';

        if (board[row][col] == ' ') {
            board[row][col] = symbols[current_player];
        } else {
            sprintf(buffer, "Mossa non valida, riprova.\n");
            send(players[current_player], buffer, strlen(buffer), 0);
            continue;
        }

        if (check_winner(symbols[current_player])) {
            sprintf(buffer, "Hai vinto!\n");
            send(players[current_player], buffer, strlen(buffer), 0);
            send(players[1 - current_player], "Hai perso!\n", 11, 0);
            break;
        }

        if (is_draw()) {
            sprintf(buffer, "Pareggio!\n");
            send(players[0], buffer, strlen(buffer), 0);
            send(players[1], buffer, strlen(buffer), 0);
            break;
        }

        current_player = 1 - current_player;
    }

    close(players[0]);
    close(players[1]);
    free(game);
    pthread_exit(NULL);
}

// Thread per gestire la lobby
void *threadLobby(void *arg) {
    Giocatore *giocatore = (Giocatore *)arg; //estraggo i dati
    char buffer[BUFFER_SIZE]; 
    memset(buffer, 0, sizeof(buffer));

    
    // Messaggio di benvenuto
    sprintf(buffer, MSG_SERVER_MENU);
    if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
        perror("[Lobby] Errore nell'invio del messaggio di benvenuto\n");
        close(giocatore->socket);
        free(giocatore);
        pthread_exit(NULL);
    }

    // Ciclo del menu della lobby
    while (1) {

        // invio il messaggio di scelta
        sprintf(buffer, MSG_CHOISE);
        if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
            perror("[Lobby] Errore nell'invio del messaggio di scelta\n");
            break;
        }
        
        // Ricevo la scelta del giocatore
        memset(buffer, 0, sizeof(buffer));
        if ( recv(giocatore->socket, buffer, sizeof(buffer), 0) <= 0 ) {
            perror("[Lobby] Errore nella ricezione della scelta del giocatore\n");
            break;
        }

        // Controllo la scelta del giocatore
        if ( strcmp( buffer, MSG_CLIENT_CREAATE ) == 0 ) { // il giocatore ha scelto di creare una partita 
            


        } else if ( strcmp( buffer, MSG_CLIENT_JOIN ) == 0 ) { // il giocatore ha scelto di unirsi a una partita



        } else if ( strcmp( buffer, MSG_CLIENT_QUIT ) == 0 ) { // il giocatore ha scelto di uscire dalla lobby e quindi dal server
            break;
        } else {
            sprintf(buffer, "[Lobby] Errore, scelta del giocatore non gestita.\n");
            continue;
        }


    }
    // Chiudi la connessione e libera la memoria
    close(giocatore->socket);
    free(giocatore);
    pthread_exit(NULL);
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if ( bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 ) {
        perror("\n\nbind fallita\n\n");
        exit(-1);
    }
    if ( listen(server_fd, MAX_CLIENTS) < 0 ) {
        perror("\n\nlisten fallita\n\n");
        exit(-1);
    }
    printf("Server in ascolto sulla porta %d...\n", PORT);

    //CICLO DI ACCETTAZIONE DELLE CONNESSIONI
    while (1) {
        int nuovaSocket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (nuovaSocket < 0) {
            perror("Errore nell'accept\n");
            continue;
        }
        printf("Un nuovo giocatore è connesso al server.\n");

        Giocatore *giocatore = malloc(sizeof(Giocatore)); //alloco il nuovo giocatore
        if (giocatore == NULL) {
            perror("Errore nell'allocazione della memoria\n");
            close(nuovaSocket);
            continue;
        }
        giocatore->socket = nuovaSocket; //salvo la socket del giocatore        
        
        //mando il giocatore nella lobby
        pthread_t thread;
        if (pthread_create(&thread, NULL, threadLobby, (void *)giocatore) != 0) {
            perror("Errore nella creazione del thread\n");
            free(giocatore);
            close(nuovaSocket);
            continue;
        }
        if ( pthread_detach(thread) != 0 ) {
            perror("Errore nel detach del thread\n");
            free(giocatore);
            close(nuovaSocket);
            continue;
        }
    }

    close(server_fd);
    return 0;
}

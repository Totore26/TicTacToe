#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "strutture.h"
#include "funzioni/funzioni.h"
#include "../Comunicazione.h"

Lobby lobby;


//dichiarazioni di funzioni
void *threadLobby(void *arg);
void *threadPartita(void *arg);

int main() {
    pthread_mutex_init(&lobby.lobbyMutex, NULL);
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

    // Ciclo della lobby (quando esco si chiude il giocatore e il thread)
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

        // il giocatore ha scelto di creare una partita 
        if ( strcmp( buffer, MSG_CLIENT_CREAATE ) == 0 ) { 

            //controllo di non aver raggiunto il numero massimo di partite
            if (!MaxPartiteRaggiunte()) {
                sprintf(buffer, "[Lobby] Errore, numero massimo di partite raggiunto.\n");
                if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Lobby] Errore nell'invio del messaggio di errore\n");
                }
                continue;
            }
            
            // creo la partita
            Partita *partita = malloc(sizeof(Partita));
            if (partita == NULL) {
                perror("[Lobby] Errore nell'allocazione della memoria per la partita\n");
                break;
            }
            // inizializzo la partita
            partita->giocatoreAdmin = *giocatore;
            partita->statoPartita = PARTITA_NUOVA_CREAZIONE; 
            partita->turnoCorrente = 0; // il primo turno è del giocatore admin
            pthread_mutex_init(&partita->partitaMutex, NULL);
            
            // aggiungo alla lobby la nuova partita
            int nuovoId = generazioneIdPartita();
            if (nuovoId == -1) {
                perror("[Lobby] Errore nella generazione dell'id della partita\n");
                free(partita);
                break;
            }
            pthread_mutex_lock(&lobby.lobbyMutex);
            lobby.partita[nuovoId] = *partita; 
            pthread_mutex_unlock(&lobby.lobbyMutex);

            // invio il messaggio di attesa al giocatore admin
            sprintf(buffer, MSG_WAITING_PLAYER);
            if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                perror("[Lobby] Errore nell'invio del messaggio di attesa secondo giocatore\n");
                break;
            }

            partita->statoPartita = PARTITA_IN_ATTESA;
            //ora bisogna solo aspettare il secondo giocatore

        // il giocatore ha scelto di unirsi a una partita
        } else if ( strcmp( buffer, MSG_CLIENT_JOIN ) == 0 ) {

            // se non ci sono partite disponibili
            if (emptyLobby) {
                sprintf(buffer, MSG_NO_GAME);
                if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Lobby] Errore nell'invio del messaggio di partite non disponibili\n");
                    break;
                }
            } else { // altrimenti invio la lista delle partite disponibili

                char *partiteDisponibili = generaStringaPartiteDisponibili(); 
                if (partiteDisponibili == NULL) {
                    perror("[Lobby] Errore nella generazione della stringa delle partite disponibili\n");
                    break;
                }
                
                // Fix format string warning by using a format specifier
                sprintf(buffer, "%s", partiteDisponibili);
                if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Lobby] Errore nell'invio della lista delle partite disponibili\n");
                    break;
                }
            }

            // adesso il giocatore deve scegliere una partita
            memset(buffer, 0, sizeof(buffer));
            if ( recv(giocatore->socket, buffer, sizeof(buffer), 0) <= 0 ) {
                perror("[Lobby] Errore nella ricezione della scelta della partita\n");
                break;
            }

            int partitaScelta = atoi(buffer);
            if ( partitaScelta < 0 || partitaScelta >= MAX_GAMES ) {
                perror("[Lobby] Errore, partita scelta non valida\n");
                break;
            }

            //creo il thread per la partita
            pthread_mutex_lock(&lobby.lobbyMutex);
            if (lobby.partita[partitaScelta].statoPartita == PARTITA_IN_ATTESA) {

                lobby.partita[partitaScelta].giocatoreGuest = *giocatore;
                lobby.partita[partitaScelta].statoPartita = PARTITA_IN_CORSO;

                pthread_t thread;
                if (pthread_create(&thread, NULL, threadPartita, (void *)&lobby.partita[partitaScelta]) != 0) {
                    perror("[Lobby] Errore nella creazione del thread per la partita\n");
                    break;
                }
                if ( pthread_detach(thread) != 0 ) { // DETACH DEL THREAD PARTITA
                    perror("[Lobby] Errore nel detach del thread per la partita\n");
                    break;
                }

            } else { // la partita non è disponibile (es. qualcuno si è unito prima)
                sprintf(buffer, MSG_JOIN_ERROR);
                if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Lobby] Errore nell'invio del messaggio di join error\n");
                    break;
                }
            }
            pthread_mutex_unlock(&lobby.lobbyMutex);

        } else if ( strcmp( buffer, MSG_CLIENT_QUIT ) == 0 ) { // il giocatore ha scelto di uscire dalla lobby e quindi dal server
            break;
        } else {
            sprintf(buffer, "[Lobby] Errore, scelta del giocatore non gestita.\n");
            break;
        }
    }
    // Chiudi la connessione e libera la memoria
    close(giocatore->socket);
    free(giocatore);
    pthread_exit(NULL);
}


// Thread per gestire una partita
void *threadPartita(void *arg) {
    Partita *partita = (Partita *)arg;
    Giocatore giocatori[2] = { partita->giocatoreAdmin, partita->giocatoreGuest };
    char buffer[1024];
    int giocatoreCorrente = 0;
    inizializzazioneGriglia(partita);

    // ciclo di gioco
    while (1) {
        
        // invio il messaggio del turno al giocatore corrente
        sprintf(buffer, MSG_YOUR_TURN);
        if ( send(giocatori[giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 ) {
            perror("[Partita] Errore nell'invio del messaggio di turno\n");
            break;
        }
        // invio all opponent il messaggio di attesa
        sprintf(buffer, MSG_SERVER_OPPONENT_TURN);
        if ( send(giocatori[1 - giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 ) {
            perror("[Partita] Errore nell'invio del messaggio di attesa all opponent\n");
            break;
        }

        // attendo la mossa del giocatore corrente
        if ( recv(giocatori[giocatoreCorrente].socket, buffer, sizeof(buffer), 0) <= 0 ) {
            perror("[Partita] Errore nella ricezione della mossa del giocatore corrente\n");
            break;
        }

        // ricevuta la mossa, aggiorno la griglia
        int row = buffer[0] - '0', col = buffer[1] - '0';

        if (partita->Griglia[row][col] == ' ') {
            partita->Griglia[row][col] = giocatoreCorrente == 0 ? 'X' : 'O';
        } else {
            sprintf(buffer, "Mossa non valida, riprova.\n");
            send(giocatori[giocatoreCorrente].socket, buffer, strlen(buffer), 0);
            continue;
        }

        if (check_winner(giocatoreCorrente == 0 ? 'X' : 'O', partita)) {
            sprintf(buffer, MSG_SERVER_WIN);
            send(giocatori[giocatoreCorrente].socket, buffer, strlen(buffer), 0);
            sprintf(buffer, MSG_SERVER_LOSE);
            send(giocatori[1 - giocatoreCorrente].socket, buffer, strlen(buffer), 0);
            break;
        }

        if (is_draw(partita)) {
            sprintf(buffer, MSG_SERVER_DRAW);
            send(giocatori[0].socket, buffer, strlen(buffer), 0);
            send(giocatori[1].socket, buffer, strlen(buffer), 0);
            break;
        }

        giocatoreCorrente = switchGiocatoreCorrente(giocatoreCorrente);
    }

    close(giocatori[0].socket);
    close(giocatori[1].socket);
    free(partita);
    pthread_exit(NULL);
}






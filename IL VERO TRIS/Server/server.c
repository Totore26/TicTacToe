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
    inizializzaStatoPartite();
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
     

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

        // Abilita il riutilizzo dell'indirizzo
        int opt = 1;
        if (setsockopt(server_fd , SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("Errore nel settaggio del socket");
            exit(EXIT_FAILURE);
        }
    
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

    // Ciclo della lobby (quando esco si chiude il giocatore e il thread) (quando entro mostra il menu)
    while (1) {

        // invio il messaggio di scelta
        sprintf(buffer, MSG_SERVER_MENU);
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
            
            Partita *partita = creaPartita(giocatore); // creo una nuova partita

            //ciclo per gestire il rematch
            while(1) {

                //CICLO IN ATTESA DI TERMINAZIONE PARTITA
                while(partita->statoPartita != PARTITA_TERMINATA){
                    sleep(1); 
                }

                //se il giocatore ha vinto
                if (giocatore->socket == partita->Vincitore) { 
                    
                    // chiedo se vuole giocare ancora 
                    sprintf(buffer, MSG_SERVER_REMATCH);
                    if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                        perror("[Lobby] Errore nell'invio del messaggio di rivincita al vincitore\n");
                        close(giocatore->socket);
                        free(giocatore);
                        pthread_exit(NULL);
                    }
                    //attendo la risposta
                    memset(buffer, 0, sizeof(buffer));
                    if ( recv(giocatore->socket, buffer, sizeof(buffer), 0) <= 0 ) {
                        perror("[Lobby] Errore nella ricezione della risposta del vincitore\n");
                        close(giocatore->socket);
                        free(giocatore);
                        pthread_exit(NULL);
                    }
                    // se il giocatore ha scelto di fare un rematch creo una nuova partita e la metto in attesa
                    if ( strcmp( buffer, MSG_CLIENT_REMATCH ) == 0 ) { // il giocatore ha scelto di fare un rematch
                        partita = creaPartita(giocatore); // creo una nuova partita
                        if (partita == NULL) {
                            perror("[Lobby] Errore nella creazione della nuova partita\n");
                            close(giocatore->socket);
                            free(giocatore);
                            pthread_exit(NULL);
                        }
                        //devo attendere di nuovo che la partita termini
                        continue;
                    } 
                    break; // il vincitore non vuole fare rematch lo porto al menu
                } 
                break; // se ha perso va direttamente al menu
            }

        } else if ( strcmp( buffer, MSG_CLIENT_JOIN ) == 0 ) { // il giocatore ha scelto di unirsi a una partita

            // se non ci sono partite disponibili
            if (emptyLobby()) {
                sprintf(buffer, MSG_NO_GAME);
                if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Lobby] Errore nell'invio del messaggio di partite non disponibili\n");
                    break;
                }
                sleep(1); // attendo un secondo prima di ripetere il ciclo
                continue;
            } else { // altrimenti invio la lista delle partite disponibili

                char *partiteDisponibili = generaStringaPartiteDisponibili(); 
                if (partiteDisponibili == NULL) {
                    perror("[Lobby] Errore nella generazione della stringa delle partite disponibili\n");
                    break;
                }
                
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
                sleep(1);
                pthread_mutex_unlock(&lobby.lobbyMutex);
                continue;
            }
            pthread_mutex_unlock(&lobby.lobbyMutex);
            
        
            // CICLO CHE ATTENDE LA TERMINAZIONE DELLA PARTITA
            // QUI DEVO GESTIRLA IN MODO IDENTICO A QUANDO CREO UNA PARTITA
            
            while(1) {

                //CICLO IN ATTESA DI TERMINAZIONE PARTITA
                while(lobby.partita[partitaScelta].statoPartita != PARTITA_TERMINATA){
                    sleep(1); 
                }

                //se il giocatore ha vinto
                if (giocatore->socket == lobby.partita[partitaScelta].Vincitore) { 
                    
                    // chiedo se vuole giocare ancora 
                    sprintf(buffer, MSG_SERVER_REMATCH);
                    if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                        perror("[Lobby] Errore nell'invio del messaggio di rivincita al vincitore\n");
                        close(giocatore->socket);
                        free(giocatore);
                        pthread_exit(NULL);
                    }
                    //attendo la risposta
                    memset(buffer, 0, sizeof(buffer));
                    if ( recv(giocatore->socket, buffer, sizeof(buffer), 0) <= 0 ) {
                        perror("[Lobby] Errore nella ricezione della risposta del vincitore\n");
                        close(giocatore->socket);
                        free(giocatore);
                        pthread_exit(NULL);
                    }
                    // se il giocatore ha scelto di fare un rematch creo una nuova partita e la metto in attesa
                    if ( strcmp( buffer, MSG_CLIENT_REMATCH ) == 0 ) { // il giocatore ha scelto di fare un rematch
                        Partita *partita = creaPartita(giocatore); // creo una nuova partita
                        if (partita == NULL) {
                            perror("[Lobby] Errore nella creazione della nuova partita\n");
                            close(giocatore->socket);
                            free(giocatore);
                            pthread_exit(NULL);
                        }
                        //devo attendere di nuovo che la partita termini
                        continue;
                    } 
                    break; // il vincitore non vuole fare rematch lo porto al menu
                } 
                break; // se ha perso va direttamente al menu
            }

            continue;

        } else if ( strcmp( buffer, MSG_CLIENT_QUIT ) == 0 ) { // il giocatore ha scelto di uscire dalla lobby e quindi dal server
            break;
        } else {
            perror("[Lobby] Errore, comando non valido\n");
            break;
        }
    }
    // Chiudo la connessione e libero la memoria
    close(giocatore->socket);
    free(giocatore);
    pthread_exit(NULL);
}


// Thread per gestire una partita
void *threadPartita(void *arg) {
    Partita *partita = (Partita *)arg;
    Giocatore giocatore[2] = { partita->giocatoreAdmin, partita->giocatoreGuest };
    char buffer[1024];
    int giocatoreCorrente = 0;
    int giocatoreInAttesa = 1;
    int contatoreTurno = -1;
    char simboloGiocatoreCorrente = 'X';
    char simboloGiocatoreInAttesa = 'O';
    char *griglia;
    inizializzazioneGriglia(partita);

    // ciclo di gioco che parla contemporaneamente con i due giocatori ( devo usare i mutex ) e ogni ciclo è un turno
    while (1) {

        contatoreTurno++;
        simboloGiocatoreCorrente = (contatoreTurno % 2 == 0) ? 'X' : 'O';
        simboloGiocatoreInAttesa = ( contatoreTurno % 2 == 0) ? 'O' : 'X';

        // avviso e invio a entrambi la griglia aggiornata
        sprintf(buffer, MSG_SERVER_BOARD);
        if ( send(giocatore[giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 || send(giocatore[giocatoreInAttesa].socket, buffer, strlen(buffer), 0) < 0 ) {
            perror("[Partita] Errore nell'invio del messaggio per la griglia iniziale\n");
            pthread_exit(NULL);
        }

        griglia = grigliaFormattata(partita->Griglia, simboloGiocatoreCorrente);
        if ( send(giocatore[giocatoreCorrente].socket, griglia, strlen(griglia), 0) < 0 ) {
            perror("[Partita] Errore nell'invio della griglia iniziale\n");
            pthread_exit(NULL);
        }   
        griglia = grigliaFormattata(partita->Griglia, simboloGiocatoreInAttesa);
        if ( send(giocatore[giocatoreInAttesa].socket, griglia, strlen(griglia), 0) < 0 ) {
            perror("[Partita] Errore nell'invio della griglia iniziale\n");
            pthread_exit(NULL);
        }

        // invio il messaggio del turno ai giocatori ( inizia sempre il proprietario cioe X )
        sprintf( buffer, MSG_YOUR_TURN );
        if ( send(giocatore[giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 ) {
            perror("[Partita] Errore nell'invio del messaggio per il turno\n");
            pthread_exit(NULL);
        }
        sprintf( buffer, MSG_OPPONENT_TURN );
        if ( send(giocatore[giocatoreInAttesa].socket, buffer, strlen(buffer), 0) < 0 ) {
            perror("[Partita] Errore nell'invio del messaggio per il turno\n");
            pthread_exit(NULL);
        }

        //ciclo fino a quando il giocatore non fa una mossa valida
        while(1) {
            // attendo la mossa 
            memset(buffer, 0, sizeof(buffer));
            if ( recv(giocatore[giocatoreCorrente].socket, buffer, sizeof(buffer), 0) <= 0 ) {
                perror("[Partita] Errore nella ricezione della mossa del giocatore corrente\n");
                close(giocatore[0].socket);
                close(giocatore[1].socket);
                partita->statoPartita = PARTITA_TERMINATA;
                pthread_exit(NULL);
            }

            // leggo l intero, lo converto in coordinate e eseguo la mossa
            int mossa = atoi(buffer);
            int *coordinate = convertiMossa(mossa);
            if (coordinate == NULL) {
                perror("[Partita] Errore nella conversione della mossa\n");
                close(giocatore[0].socket);
                close(giocatore[1].socket);
                partita->statoPartita = PARTITA_TERMINATA;
                pthread_exit(NULL);
            }

            if ( eseguiMossa(partita->Griglia, coordinate[0], coordinate[1], simboloGiocatoreCorrente) == 0 ) { //se la mossa non è valida
                //avviso il client
                sprintf(buffer, MSG_INVALID_MOVE);
                if ( send(giocatore[giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Partita] Errore nell'invio del messaggio di mossa non valida\n");
                    free(coordinate);
                    close(giocatore[0].socket);
                    close(giocatore[1].socket);
                    partita->statoPartita = PARTITA_TERMINATA;
                    pthread_exit(NULL);
                } 
                continue; 
            }
            break; // esco dal ciclo se la mossa è valida
        }

        // controllo se c'è un pareggio

        if( contatoreTurno == 8 ) {
            if( is_draw(partita) ) {
                sprintf(buffer, MSG_SERVER_DRAW); // invio a entrambi i giocatori il messaggio di pareggio
                if ( send(partita->giocatoreAdmin.socket, buffer, strlen(buffer), 0) < 0 || send(partita->giocatoreGuest.socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Partita] Errore nell'invio del messaggio di pareggio\n");
                    close(giocatore[0].socket);
                    close(giocatore[1].socket);
                    partita->statoPartita = PARTITA_TERMINATA;
                    pthread_exit(NULL);
                }
                sleep(1); // attendo un secondo prima di chiedere il rematch

                //controllo se proprietario vuole rematch
                sprintf(buffer, MSG_SERVER_REMATCH);
                if ( send(partita->giocatoreAdmin.socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Partita] Errore nell'invio del messaggio di rivincita al Proprietario\n");
                    close(giocatore[0].socket);
                    close(giocatore[1].socket);
                    partita->statoPartita = PARTITA_TERMINATA;
                    pthread_exit(NULL);
                }
                sprintf(buffer, MSG_WAITING_REMATCH);
                if ( send(partita->giocatoreGuest.socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Partita] Errore nell'invio del messaggio di attesa al Guest\n");
                    close(giocatore[0].socket);
                    close(giocatore[1].socket);
                    partita->statoPartita = PARTITA_TERMINATA;
                    pthread_exit(NULL);
                }

                //attendo la risposta del proprietario
                memset(buffer, 0, sizeof(buffer));
                if ( recv(partita->giocatoreAdmin.socket, buffer, sizeof(buffer), 0) <= 0 ) {
                    perror("[Partita] Errore nella ricezione della risposta del proprietario\n");
                    close(giocatore[0].socket);
                    close(giocatore[1].socket);
                    partita->statoPartita = PARTITA_TERMINATA;
                    pthread_exit(NULL);
                }

                if ( strcmp( buffer, MSG_CLIENT_QUIT ) == 0 ) { // il proprietario ha scelto di uscire tornano entrambi al menu
                    // informo il guest che il proprietario ha abbandonato
                    sprintf(buffer, MSG_SERVER_ADMIN_QUIT);
                    if ( send(partita->giocatoreGuest.socket, buffer, strlen(buffer), 0) < 0 ) {
                        perror("[Partita] Errore nell'invio del messaggio di abbandono del proprietario\n");
                        close(giocatore[0].socket);
                        close(giocatore[1].socket);
                        partita->statoPartita = PARTITA_TERMINATA;
                        pthread_exit(NULL);
                    }
                    // i due giocatori tornano al menu
                    sleep(1);
                    partita->Vincitore = -1;
                    partita->statoPartita = PARTITA_TERMINATA;
                    pthread_exit(NULL);
                } else if ( strcmp( buffer, MSG_CLIENT_REMATCH ) == 0 ) { // il proprietario ha scelto di fare un rematch quindi informo il guest
                    
                    //chiedo il rematch al guest
                    sprintf(buffer, MSG_SERVER_REMATCH);
                    if ( send(partita->giocatoreGuest.socket, buffer, strlen(buffer), 0) < 0 ) {
                        perror("[Partita] Errore nell'invio del messaggio di rivincita al Guest\n");
                        close(giocatore[0].socket);
                        close(giocatore[1].socket);
                        partita->statoPartita = PARTITA_TERMINATA;
                        pthread_exit(NULL);
                    }

                    //attendo la risposta del guest
                    memset(buffer, 0, sizeof(buffer));
                    if ( recv(partita->giocatoreGuest.socket, buffer, sizeof(buffer), 0) <= 0 ) {
                        perror("[Partita] Errore nella ricezione della risposta del guest\n");
                        close(giocatore[0].socket);
                        close(giocatore[1].socket);
                        partita->statoPartita = PARTITA_TERMINATA;
                        pthread_exit(NULL);
                    }

                    if (strcmp(buffer, MSG_CLIENT_QUIT) == 0) { // il guest ha scelto di uscire
                        // informo il proprietario che il guest ha abbandonato
                        sprintf(buffer, MSG_SERVER_ADMIN_QUIT);
                        if ( send(partita->giocatoreAdmin.socket, buffer, strlen(buffer), 0) < 0 ) {
                            perror("[Partita] Errore nell'invio del messaggio di abbandono del guest\n");
                            close(giocatore[0].socket);
                            close(giocatore[1].socket);
                            partita->statoPartita = PARTITA_TERMINATA;
                            pthread_exit(NULL);
                        }
                        sleep(1); // attendo un secondo prima di chiudere la partita
                        partita->Vincitore = -1;
                        partita->statoPartita = PARTITA_TERMINATA;
                        pthread_exit(NULL);

                    } else if (strcmp(buffer, MSG_CLIENT_REMATCH) == 0) { // il guest ha scelto di fare un rematch quindi ricomincia la partita corrente
                        inizializzazioneGriglia(partita);
                        giocatoreCorrente = 0;
                        giocatoreInAttesa = 1;
                        contatoreTurno = -1;
                        continue; // ricomincia la partita
                    }

                } else {
                    perror("[Partita] Errore, comando non valido\n");
                    close(giocatore[0].socket);
                    close(giocatore[1].socket);
                    partita->statoPartita = PARTITA_TERMINATA;
                    pthread_exit(NULL);
                }
            }
        }

        //controllo se il giocatore corrente ha vinto

        if ( check_winner(simboloGiocatoreCorrente, partita) ) { // se il giocatore corrente ha vinto

            // salvo il vincitore
            partita->Vincitore = giocatore[giocatoreCorrente].socket;
            partita->giocatoreAdmin = giocatore[giocatoreCorrente];

            //invio al vincitore il messaggio di vittoria
            sprintf(buffer, MSG_SERVER_WIN);
            if ( send(giocatore[giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 ) {
                perror("[Partita] Errore nell'invio del messaggio di vittoria\n");
                close(giocatore[0].socket);
                close(giocatore[1].socket);
                partita->statoPartita = PARTITA_TERMINATA;
                pthread_exit(NULL);
            }
            //invio al perdente il messaggio di sconfitta
            sprintf(buffer, MSG_SERVER_LOSE);
            if ( send(giocatore[giocatoreInAttesa].socket, buffer, strlen(buffer), 0) < 0 ) {
                perror("[Partita] Errore nell'invio del messaggio di sconfitta\n");
                close(giocatore[0].socket);
                close(giocatore[1].socket);
                partita->statoPartita = PARTITA_TERMINATA;
                pthread_exit(NULL);
            }
            //il vincitore potra scegliere se fare un altra partita nel thread della lobby 
            partita->statoPartita = PARTITA_TERMINATA;
            pthread_exit(NULL);
        }

        giocatoreCorrente = switchGiocatore(giocatoreCorrente);
        giocatoreInAttesa = switchGiocatore(giocatoreInAttesa);
    }

    close(giocatore[0].socket);
    close(giocatore[1].socket);
    partita->statoPartita = PARTITA_TERMINATA;
    pthread_exit(NULL);
    
}






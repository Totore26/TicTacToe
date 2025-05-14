#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

#include "strutture.h"
#include "funzioni.h"
#include "Comunicazione.h"

#define UNUSED(x) (void)(x)
#define MAX_NAME_LENGTH 50


Lobby lobby;
Giocatori giocatori;



//dichiarazioni di funzioni
void *threadLobby(void *arg);
void *threadPartita(void *arg);

void sigHandler(int signum) {
    UNUSED(signum);  // Properly mark parameter as unused
    
    printf("\nServer in chiusura...\n");
    // Chiudo tutte le partite in corso
    for (int i = 0; i < MAX_GAMES; i++) {
        if (lobby.partita[i].statoPartita != PARTITA_TERMINATA) {
            lobby.partita[i].statoPartita = PARTITA_TERMINATA;
        }
        // Chiudo le socket dei giocatori
        if (lobby.partita[i].giocatoreAdmin.socket != -1) {
            close(lobby.partita[i].giocatoreAdmin.socket);
            lobby.partita[i].giocatoreAdmin.socket = -1;
        }
        if (lobby.partita[i].giocatoreGuest.socket != -1) {
            close(lobby.partita[i].giocatoreGuest.socket);
            lobby.partita[i].giocatoreGuest.socket = -1;
        }
    }

    pthread_mutex_destroy(&lobby.lobbyMutex);
    printf("Server chiuso correttamente.\n");
    exit(0);
}

int main() {

    //Gestione dei signali interrump and terminate
    // per chiudere il server in modo pulito
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    inizializzaStatoPartite();
    inizializzaGiocatori();

    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
     

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Abilita il riutilizzo dell'indirizzo e della porta
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Ascolta su qualsiasi interfaccia
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
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

        giocatore->id = assegnazioneGiocatore(*giocatore); //inserisce il giocatore nella lista dei giocatori connessi
        
        
        
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


    // Ricevi il nome del giocatoreù
    char nome[MAX_NAME_LENGTH];
    while (1) { // Ciclo per assicurarsi che il nome sia valido
        memset(nome, 0, sizeof(nome));
        printf("\n\n\nAttendo il nome del giocatore...\n");

        if (recv(giocatore->socket, nome, sizeof(nome), 0) <= 0) {
            perror("Errore nella ricezione del nome del giocatore");
            close(giocatore->socket);
            free(giocatore);
            pthread_exit(NULL);
        }
    
        // Controlla se il nome è vuoto
        if (strlen(nome) == 0) {
            send(giocatore->socket, MSG_SERVER_INVALID_NAME_ERROR, strlen(MSG_SERVER_INVALID_NAME_ERROR), 0);
            continue; // Torna all'inizio del ciclo per chiedere un nuovo nome
        }
    
        // Controlla se il nome è già in uso
        if (nomeDuplicato(nome)) {
            send(giocatore->socket, MSG_SERVER_INVALID_NAME_ERROR, strlen(MSG_SERVER_INVALID_NAME_ERROR), 0);
            continue; // Torna all'inizio del ciclo per chiedere un nuovo nome
        }
    
        // Nome valido, aggiungilo all'array e esci dal ciclo
        aggiungiNome(nome);
        strcpy(giocatore->nome, nome);
        send(giocatore->socket, MSG_SERVER_REGISTRATION_OK, strlen(MSG_SERVER_REGISTRATION_OK), 0);

        // Notifica tutti i client nel menu principale della nuova registrazione
        for (int i = 0; i < MAX_CLIENTS; i++) {
                if (giocatori.giocatore[i].socket != -1 &&
                    giocatori.giocatore[i].stato == 2 && // nel menu principale
                    giocatori.giocatore[i].socket != giocatore->socket) // non notificare se stesso
                {
                    char notify[128];
                    printf("[DEBUG] Invio notifica di nuova registrazione a %s (socket %d)\n", giocatori.giocatore[i].nome, giocatori.giocatore[i].socket);
                    snprintf(notify, sizeof(notify), "%s:%s", MSG_NEW_USER_REGISTERED , giocatore->nome);
                    send(giocatori.giocatore[i].socket, notify, strlen(notify), 0);
                }
            }            
            
            
            break;
    }

    usleep(100000); // attendo un secondo prima di continuare

    // Ciclo della lobby (quando esco si chiude il giocatore e il thread) (quando entro mostra il menu)
    while (1) {


        giocatori.giocatore[giocatore->id].stato = 2; // metto lo stato a 2 (nel menu principale)

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
        
        giocatori.giocatore[giocatore->id].stato = 0; // metto lo stato a 0 (non nel menu principale)

        // il giocatore ha scelto di creare una partita 
        if ( strcmp( buffer, MSG_CLIENT_CREAATE ) == 0 ) { 
            
                //controllo di non aver raggiunto il numero massimo di partite (se raggiunte torna 1)
                if (MaxPartiteRaggiunte()) {
                    perror("[Lobby] Errore, numero massimo di partite raggiunto, informo il client\n");
                    sprintf(buffer, MSG_SERVER_MAX_GAMES);
                    if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                        perror("[Lobby] Errore nell'invio del messaggio di errore\n");
                    }
                    return NULL;
                }
                
                // creo la partita
                Partita *partita = malloc(sizeof(Partita));
                if (partita == NULL) {
                    perror("[Lobby] Errore nell'allocazione della memoria per la partita\n");
                    return NULL;
                }
                // inizializzo la partita
                partita->giocatoreAdmin = *giocatore;
                partita->statoPartita = PARTITA_IN_ATTESA;
                
                // aggiungo alla lobby la nuova partita
                int nuovoId = generazioneIdPartita();
                if (nuovoId == -1) {
                    perror("[Lobby] Errore nella generazione dell'id della partita\n");
                    partita->statoPartita = PARTITA_TERMINATA;
                    return NULL;
                }

                pthread_mutex_lock(&lobby.lobbyMutex);
                lobby.partita[nuovoId] = *partita; 
                pthread_mutex_unlock(&lobby.lobbyMutex);

                // invio il messaggio di attesa al giocatore admin
                sprintf(buffer, MSG_WAITING_PLAYER);
                if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Lobby] Errore nell'invio del messaggio di attesa secondo giocatore\n");
                    return NULL;
                }
                
                // invio a tutti i client che guardano la lista delle partite disponibili quella aggiornata con la nuova partita:
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (giocatori.giocatore[i].socket != -1 && giocatori.giocatore[i].stato == 1) {
                        // invio il messaggio di lista aggiornata di partite disponibili
                        sprintf(buffer, MSG_SERVER_GAME_LIST);
                        if ( send(giocatori.giocatore[i].socket, buffer, strlen(buffer), 0) < 0 ) {
                            perror("[Lobby] Errore nell'invio del messaggio di lista aggiornata di partite disponibili\n");
                            return NULL;
                        }
        
                        usleep(100000); // attendo un secondo prima di inviare il messaggio
        
                        char *partiteDisponibili = generaStringaPartiteDisponibili();
                        if (partiteDisponibili == NULL) {
                            perror("[Lobby] Errore nella generazione della stringa delle partite disponibili\n");
                            return NULL;
                        }
                        sprintf(buffer, "%s", partiteDisponibili);
                        if ( send(giocatori.giocatore[i].socket, buffer, strlen(buffer), 0) < 0 ) {
                            perror("[Lobby] Errore nell'invio della lista delle partite disponibili\n");
                            free(partiteDisponibili);
                            return NULL;
                        }
                    }
                }

            //ciclo per gestire il rematch
            while(1) {

                while(lobby.partita[nuovoId].statoPartita != PARTITA_TERMINATA){
                    // Verifica se il client è ancora connesso
                    char testBuf[1];
                    int result = recv(giocatore->socket, testBuf, 1, MSG_PEEK | MSG_DONTWAIT);
                    if(result == 0 || (result == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                        // Client disconnesso
                        printf("[Lobby] Client disconnesso mentre era in attesa\n");
                        pthread_mutex_lock(&lobby.lobbyMutex);
                        lobby.partita[nuovoId].statoPartita = PARTITA_TERMINATA;
                        pthread_mutex_unlock(&lobby.lobbyMutex);
                        return NULL;
                    }
                    sleep(1);
                }
                
                //se il giocatore ha vinto
                if (giocatore->socket == lobby.partita[nuovoId].Vincitore) { 
                    
                    printf("Il giocatore %d ha vinto la partita\n", giocatore->socket);

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
                        
                        // invio a tutti i client che guardano la lista delle partite disponibili quella aggiornata con la nuova partita:
                        for (int i = 0; i < MAX_CLIENTS; i++) {
                            if (giocatori.giocatore[i].socket != -1 && giocatori.giocatore[i].stato == 1) {
                                // invio il messaggio di lista aggiornata di partite disponibili
                                sprintf(buffer, MSG_SERVER_GAME_LIST);
                                if ( send(giocatori.giocatore[i].socket, buffer, strlen(buffer), 0) < 0 ) {
                                    perror("[Lobby] Errore nell'invio del messaggio di lista aggiornata di partite disponibili\n");
                                    return NULL;
                                }
                
                                usleep(100000); // attendo un secondo prima di inviare il messaggio
                
                                char *partiteDisponibili = generaStringaPartiteDisponibili();
                                if (partiteDisponibili == NULL) {
                                    perror("[Lobby] Errore nella generazione della stringa delle partite disponibili\n");
                                    return NULL;
                                }
                                sprintf(buffer, "%s", partiteDisponibili);
                                if ( send(giocatori.giocatore[i].socket, buffer, strlen(buffer), 0) < 0 ) {
                                    perror("[Lobby] Errore nell'invio della lista delle partite disponibili\n");
                                    free(partiteDisponibili);
                                    return NULL;
                                }
                            }
                        }


                        //devo attendere di nuovo che la partita termini
                        continue;
                    } 
                    break; // il vincitore non vuole fare rematch lo porto al menu
                } 
                break; // se ha perso va direttamente al menu
            }

        } else if ( strcmp( buffer, MSG_CLIENT_JOIN ) == 0 ) { // il giocatore ha scelto di unirsi a una partita
            
            //prima metto lo stato del giocatore a 1 cosi da inviargli eventuali liste aggiornate di partite
            giocatori.giocatore[giocatore->id].stato = 1;

            printf("[Lobby] Il giocatore %d ha scelto di unirsi a una partita, modifico il suo stato a 1\n", giocatore->socket);

            // se non ci sono partite disponibili
            if (emptyLobby()) {
                sprintf(buffer, MSG_NO_GAME);
                if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                    perror("[Lobby] Errore nell'invio del messaggio di partite non disponibili\n");
                    break;
                }
                giocatori.giocatore[giocatore->id].stato = 0; // metto lo stato a 0
                usleep(100000); // attendo un secondo prima di ripetere il ciclo
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

            if ( strcmp(buffer, MSG_CLIENT_QUIT) == 0 ) { 
                // il giocatore ha scelto di tornare al menù principale (metto lo stato a 0)
                giocatori.giocatore[giocatore->id].stato = 0;
                continue;
            }

            int partitaScelta = atoi(buffer);
            if ( partitaScelta < 0 || partitaScelta >= MAX_GAMES ) {
                perror("[Lobby] Errore, partita scelta non valida\n");
                break;
            }

                //creo il thread per la partita
                pthread_mutex_lock(&lobby.lobbyMutex);
                if (lobby.partita[partitaScelta].statoPartita == PARTITA_IN_ATTESA) {

                    giocatori.giocatore[giocatore->id].stato = 0; // metto lo stato a 0

                    sprintf(buffer, "%s",MSG_SERVER_JOIN_REQUEST);
                    if ( send(lobby.partita[partitaScelta].giocatoreAdmin.socket, buffer, strlen(buffer), 0) < 0 ) {
                        perror("[Lobby] Errore nell'invio della lista delle partite disponibili\n");
                        break;
                    }
    
                    //ricevo il messaggio ri risposta
                    memset(buffer, 0, sizeof(buffer));
                    if ( recv(lobby.partita[partitaScelta].giocatoreAdmin.socket, buffer, sizeof(buffer), 0) <= 0 ) {
                        perror("[Lobby] Errore nella ricezione della risposta del giocatore admin\n");
                        close(lobby.partita[partitaScelta].giocatoreAdmin.socket);
                        //send MSG_SERVER_REFUSE
                        sprintf(buffer, MSG_SERVER_ADMIN_QUIT);
                        if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                            perror("[Lobby] Errore nell'invio del messaggio di rifiuto\n");
                            close(giocatore->socket);
                            free(giocatore);
                            pthread_exit(NULL);
                        }
                        usleep(500000);
                        //elimino la partita dalla lobby
                        lobby.partita[partitaScelta].statoPartita = PARTITA_TERMINATA;
                        pthread_mutex_unlock(&lobby.lobbyMutex); // Sblocco il mutex prima di tornare al menu
                        continue; // Torna al menu principale
                        break;
                    }

                    if ( strcmp(buffer, MSG_CLIENT_ACCEPT) == 0 ) { 


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
                    }else if ( strcmp(buffer, MSG_CLIENT_REFUSE) == 0 ) { // il giocatore ha rifiutato la richiesta di join

                        //mando messaggio di rifiuto al giocatore
                        sprintf(buffer, MSG_SERVER_REFUSE);
                        if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                            perror("[Lobby] Errore nell'invio del messaggio di rifiuto\n");
                            close(giocatore->socket);
                            free(giocatore);
                            pthread_exit(NULL);
                        }
                        usleep(500000);
                        pthread_mutex_unlock(&lobby.lobbyMutex); // Sblocco il mutex prima di tornare al menu
                        continue; // Torna al menu principale
                    }

                } else { // la partita non è disponibile (es. qualcuno si è unito prima)
                    giocatori.giocatore[giocatore->id].stato = 0;
                    sprintf(buffer, MSG_JOIN_ERROR);
                    if ( send(giocatore->socket, buffer, strlen(buffer), 0) < 0 ) {
                        perror("[Lobby] Errore nell'invio del messaggio di join error\n");
                        break;
                    }
                    usleep(100000);
                    pthread_mutex_unlock(&lobby.lobbyMutex);
                    continue;
                }
                pthread_mutex_unlock(&lobby.lobbyMutex);

                giocatori.giocatore[giocatore->id].stato = 0;
                
            
                // CICLO CHE ATTENDE LA TERMINAZIONE DELLA PARTITA
                // QUI DEVO GESTIRLA IN MODO IDENTICO A QUANDO CREO UNA PARTITA
                
                while(1) {

                    //CICLO IN ATTESA DI TERMINAZIONE PARTITA
                    while(lobby.partita[partitaScelta].statoPartita != PARTITA_TERMINATA){
                        sleep(1); 
                    }

                    //se il giocatore ha vinto
                    if (giocatore->socket == lobby.partita[partitaScelta].Vincitore) { 
                        
                        usleep(100000); // attendo un secondo prima di inviare il messaggio
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

                        // invio a tutti i client che guardano la lista delle partite disponibili quella aggiornata con la nuova partita:
                        for (int i = 0; i < MAX_CLIENTS; i++) {
                            if (giocatori.giocatore[i].socket != -1 && giocatori.giocatore[i].stato == 1) {
                                // invio il messaggio di lista aggiornata di partite disponibili
                                sprintf(buffer, MSG_SERVER_GAME_LIST);
                                if ( send(giocatori.giocatore[i].socket, buffer, strlen(buffer), 0) < 0 ) {
                                    perror("[Lobby] Errore nell'invio del messaggio di lista aggiornata di partite disponibili\n");
                                    return NULL;
                                }
                
                                usleep(100000); // attendo un secondo prima di inviare il messaggio
                
                                char *partiteDisponibili = generaStringaPartiteDisponibili();
                                if (partiteDisponibili == NULL) {
                                    perror("[Lobby] Errore nella generazione della stringa delle partite disponibili\n");
                                    return NULL;
                                }
                                sprintf(buffer, "%s", partiteDisponibili);
                                if ( send(giocatori.giocatore[i].socket, buffer, strlen(buffer), 0) < 0 ) {
                                    perror("[Lobby] Errore nell'invio della lista delle partite disponibili\n");
                                    free(partiteDisponibili);
                                    return NULL;
                                }
                            }
                        }

                            //devo attendere di nuovo che la partita termini
                            continue;
                        } else {
                            if ( strcmp( buffer, MSG_CLIENT_QUIT ) == 0 )
                                break; // il vincitore non vuole fare rematch lo porto al menu
                        }
                    } 
                    break; // se ha perso va direttamente al menu
                }
            
                usleep(100000); // attendo un secondo prima di inviare il messaggio
                
        

            continue;

        } else if ( strcmp( buffer, MSG_CLIENT_QUIT ) == 0 ) { // il giocatore ha scelto di uscire dalla lobby e quindi dal server
            break;
        } else {
            perror("[Lobby] Errore, comando non valido\n");
            break;
        }
    }
    // Chiudo la connessione e libero la memoria
    rimuoviNome(giocatore->nome);
    giocatori.giocatore[giocatore->id].socket = -1; // rimuovo il giocatore dalla lista dei giocatori connessi

    // Notifica tutti i client nel menu principale della disconnessione
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (giocatori.giocatore[i].socket != -1 &&
            giocatori.giocatore[i].stato == 2 && // nel menu principale
            giocatori.giocatore[i].socket != giocatore->socket) // non notificare se stesso
        {
            char notify[128];
            printf("[DEBUG] Invio notifica di disconnessione di %s (socket %d)\n", giocatore->nome, giocatori.giocatore[i].socket);
            snprintf(notify, sizeof(notify), "%s:%s", MSG_USER_DISCONNECTED, giocatore->nome);
            send(giocatori.giocatore[i].socket, notify, strlen(notify), 0);
        }
    }
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

         // avviso e invio a entrambi la griglia aggiornata
        sprintf(buffer, MSG_SERVER_START);
        if ( send(giocatore[giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 || send(giocatore[giocatoreInAttesa].socket, buffer, strlen(buffer), 0) < 0 ) {
             perror("[Partita] Errore nell'invio del messaggio per la griglia iniziale\n");
            pthread_exit(NULL);
        }
        usleep(100000);

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
        usleep(100000); // attendo un secondo prima di inviare la griglia

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
        usleep(100000); // attendo un secondo prima di inviare il messaggio del turno
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
                // Il giocatore corrente si è disconnesso, notifico l'altro giocatore
                sprintf(buffer, MSG_SERVER_OPPONENT_LEFT);
                send(giocatore[giocatoreInAttesa].socket, buffer, strlen(buffer), 0);
                close(giocatore[giocatoreCorrente].socket);
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

    
        //controllo se il giocatore corrente ha vinto

        if ( check_winner(simboloGiocatoreCorrente, partita) ) { // se il giocatore corrente ha vinto

            // avviso e invio a entrambi la griglia aggiornata
            sprintf(buffer, MSG_SERVER_BOARD);
            if ( send(giocatore[giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 || send(giocatore[giocatoreInAttesa].socket, buffer, strlen(buffer), 0) < 0 ) {
                perror("[Partita] Errore nell'invio del messaggio per la griglia iniziale\n");
                pthread_exit(NULL);
            }
            usleep(100000); // attendo un secondo prima di inviare la griglia

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
            usleep(100000); // attendo un secondo prima di inviare il messaggio del turno
            // invio il messaggio del turno ai giocatori ( inizia sempre il proprietario cioe X )
        

            // salvo il vincitore
            partita->Vincitore = giocatore[giocatoreCorrente].socket;
            partita->giocatoreAdmin = giocatore[giocatoreCorrente];

            //invio al vincitore il messaggio di vittoria
            sprintf(buffer, MSG_SERVER_WIN);
            if ( send(giocatore[giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 ) {
                // Il vincitore si è disconnesso, notifico il perdente
                sprintf(buffer, MSG_SERVER_OPPONENT_LEFT); 
                send(giocatore[giocatoreInAttesa].socket, buffer, strlen(buffer), 0);
                close(giocatore[giocatoreCorrente].socket);
                partita->statoPartita = PARTITA_TERMINATA;
                pthread_exit(NULL);
            }
            //invio al perdente il messaggio di sconfitta
            sprintf(buffer, MSG_SERVER_LOSE);
            if ( send(giocatore[giocatoreInAttesa].socket, buffer, strlen(buffer), 0) < 0 ) {
                // Il perdente si è disconnesso, notifico il vincitore
                sprintf(buffer, MSG_SERVER_OPPONENT_LEFT);
                send(giocatore[giocatoreCorrente].socket, buffer, strlen(buffer), 0);
                close(giocatore[giocatoreInAttesa].socket);
                partita->statoPartita = PARTITA_TERMINATA; 
                pthread_exit(NULL);
            }
            
            usleep(100000); // attendo un secondo prima di chiedere il rematch
            //il vincitore potra scegliere se fare un altra partita nel thread della lobby 
            partita->statoPartita = PARTITA_TERMINATA;
            pthread_exit(NULL);
        }


            // controllo se c'è un pareggio

            if( contatoreTurno == 8 ) {
                if( is_draw(partita) ) {

                     // avviso e invio a entrambi la griglia aggiornata
                    sprintf(buffer, MSG_SERVER_BOARD);
                    if ( send(giocatore[giocatoreCorrente].socket, buffer, strlen(buffer), 0) < 0 || send(giocatore[giocatoreInAttesa].socket, buffer, strlen(buffer), 0) < 0 ) {
                        perror("[Partita] Errore nell'invio del messaggio per la griglia iniziale\n");
                        pthread_exit(NULL);
                    }
                    usleep(100000); // attendo un secondo prima di inviare la griglia

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
                    usleep(100000); // attendo un secondo prima di inviare il messaggio del turno
                    // invio il messaggio del turno ai giocatori ( inizia sempre il proprietario cioe X )


                    sprintf(buffer, MSG_SERVER_DRAW); // invio a entrambi i giocatori il messaggio di pareggio
                    if ( send(partita->giocatoreAdmin.socket, buffer, strlen(buffer), 0) < 0 || send(partita->giocatoreGuest.socket, buffer, strlen(buffer), 0) < 0 ) {
                        perror("[Partita] Errore nell'invio del messaggio di pareggio\n");
                        close(giocatore[0].socket);
                        close(giocatore[1].socket);
                        partita->statoPartita = PARTITA_TERMINATA;
                        pthread_exit(NULL);
                    }
                    usleep(100000); // attendo un secondo prima di chiedere il rematch
    
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
                        sprintf(buffer, MSG_SERVER_ADMIN_QUIT);
                            if ( send(partita->giocatoreGuest.socket, buffer, strlen(buffer), 0) < 0 ) {
                                perror("[Partita] Errore nell'invio del messaggio di abbandono del guest\n");
                                close(giocatore[0].socket);
                                close(giocatore[1].socket);
                                partita->statoPartita = PARTITA_TERMINATA;
                                pthread_exit(NULL);
                            }
                        close(partita->giocatoreAdmin.socket);
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
                        usleep(100000);
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
                            sprintf(buffer, MSG_SERVER_ADMIN_QUIT);
                            if ( send(partita->giocatoreAdmin.socket, buffer, strlen(buffer), 0) < 0 ) {
                                perror("[Partita] Errore nell'invio del messaggio di abbandono del guest\n");
                                close(giocatore[0].socket);
                                close(giocatore[1].socket);
                                partita->statoPartita = PARTITA_TERMINATA;
                                pthread_exit(NULL);
                            }
                            perror("[Partita] Errore nella ricezione della risposta del guest\n");
                            close(partita->giocatoreGuest.socket);
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
                            usleep(100000); // attendo un secondo prima di chiudere la partita
                            partita->Vincitore = -1;
                            partita->statoPartita = PARTITA_TERMINATA;
                            pthread_exit(NULL);
    
                        } else if (strcmp(buffer, MSG_CLIENT_REMATCH) == 0) { // il guest ha scelto di fare un rematch quindi ricomincia la partita corrente
                            sprintf(buffer, MSG_SERVER_REMATCH);
                            if ( send(partita->giocatoreAdmin.socket, buffer, strlen(buffer), 0) < 0 ) {
                                perror("[Partita] Errore nell'invio del messaggio di abbandono del guest\n");
                                close(giocatore[0].socket);
                                close(giocatore[1].socket);
                                partita->statoPartita = PARTITA_TERMINATA;
                                pthread_exit(NULL);
                            }
                            inizializzazioneGriglia(partita);
                            giocatoreCorrente = 0;
                            giocatoreInAttesa = 1;
                            contatoreTurno = -1;
                            usleep(100000); // attendo un secondo prima di ricominciare la partita
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
    

        giocatoreCorrente = switchGiocatore(giocatoreCorrente);
        giocatoreInAttesa = switchGiocatore(giocatoreInAttesa);
    }

    close(giocatore[0].socket);
    close(giocatore[1].socket);
    partita->statoPartita = PARTITA_TERMINATA;
    pthread_exit(NULL);
    
}
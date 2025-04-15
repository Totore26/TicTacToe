#include "funzioni.h"
#include "strutture.h"
#include <sys/time.h>



// ==========================================================
//  FUNZIONE PER CONNESSIONE CON IL SERVER  
// ==========================================================



void inizializza_socket()
{
    struct sockaddr_in ser_add;
    socklen_t lenght = sizeof(struct sockaddr_in);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket creation error"), exit(EXIT_FAILURE);

    const int opt = 1;

    struct timeval timer;
    timer.tv_sec = 300;  // Timer di 300 secondi
    timer.tv_usec = 0;

    //timer per invio e ricezione
    if (setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &timer, sizeof(timer)) < 0 || setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer)) < 0)
        perror("set socket timer error"), exit(EXIT_FAILURE);

    //disattiva bufferizzazione socket
    if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0)
        perror("tcp nodelay error"), exit(EXIT_FAILURE);

    memset(&ser_add, 0, sizeof(ser_add));
    ser_add.sin_family = AF_INET;
    ser_add.sin_port = htons(8080);
    ser_add.sin_addr.s_addr = inet_addr("127.0.0.1");

    //non c'è bisogno di inizializzare manualmente la porte client e fare il bind

    if (connect(sd, (struct sockaddr *)&ser_add, lenght) < 0)
        perror("connect error"), exit(EXIT_FAILURE);
}



// ==========================================================
//  FUNZIONE PER GESTIRE IL MENU 
// ==========================================================

void funzione_menu() {
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];
    
    while (1) {

        CLEAR_SCREEN();
        printf("\n");
        printf("╔════════════════════════════════╗\n");
        printf("║        MENU PRINCIPALE         ║\n");
        printf("╠════════════════════════════════╣\n");
        printf("║  1. Crea partita               ║\n");
        printf("║  2. Unisciti a partita         ║\n");
        printf("║  3. Esci                       ║\n");
        printf("╚════════════════════════════════╝\n");
        printf(" Scegli un'opzione-> ");
        // Inizializza il buffer
        
        
        
        memset(input, 0, MAXSCRITTORE);  // Fixed buffer size
        if (fgets(input, MAXSCRITTORE, stdin) == NULL) {  // Fixed buffer size
            continue;
        }
        
        // Rimuove il newline
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "1")==0) {
            // Invia richiesta di creazione partita
            send(sd, MSG_CLIENT_CREAATE, strlen(MSG_CLIENT_CREAATE), 0);
            funzione_crea_partita();
            break;
        } else if (strcmp(input, "2")==0) {
            // Invia richiesta di unione a partita
            send(sd, MSG_CLIENT_JOIN, strlen(MSG_CLIENT_JOIN), 0);
            funzione_entra_partita();
            break;
        } else if (strcmp(input, "3")==0) {
            // Invia richiesta di uscita
            send(sd, MSG_CLIENT_QUIT, strlen(MSG_CLIENT_QUIT), 0);
            close(sd);
            exit(EXIT_SUCCESS);
            } else {
                printf("Scelta non valida. Riprova...\n");
                sleep(1); // Aspetta 1 secondo prima di ripetere il menu
            }
    } // Close the while loop

}

// ==========================================================
//  FUNZIONE PER GESTIRE LA CREAZIONE DELLA PARTITA
// ==========================================================


void funzione_crea_partita(){
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];

    // Ricevi messaggio dal server
    memset(buffer, 0, MAXLETTORE);
    if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
        printf("Connessione al server persa.\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(buffer, MSG_SERVER_MAX_GAMES)== 0) {
        // Se il server ha restituito "MSG_SERVER_MAX_GAMES", significa che sono state raggiunte le partite massime
        printf("Massimo numero di partite raggiunto. Torna al menu principale.\n");
        return;
    }
    if (strcmp(buffer, MSG_WAITING_PLAYER) == 0) {
        // Se il server ha restituito "MSG_WAITING_PLAYER", significa che la partita è in attesa di un avversario
        // Mostra il messaggio ricevuto
        CLEAR_SCREEN();
        printf("Aspettando un avversario...\n");


    while (1)
    {
        //Quando ricevo MSG_SERVER_START allora il client inizia la partita da proprietario della lobby
        sleep(1);
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }
        
        if (strcmp(buffer, MSG_SERVER_START)==0){
            //Adesso ripulisco lo standard input
            gioca_partita(PROPRIETARIO);
            return;
        }
    }
    }
    

}


// ==========================================================
//  FUNZIONE PER LA SCELTA DELLA PARTITA 
// ==========================================================


// Funzione per ottenere la mossa del client
int get_valid_match(char *input) {
    while (1) {
        printf("\nScegli una partita disponibile (0-9) \nOppure premi 'q' per tornare al menù principale:\n");
        // Legge l'input dell'utente
        // Se fgets restituisce NULL, significa che c'è stato un errore o EOF
        // In tal caso, continua il ciclo per chiedere di nuovo l'input        
        if (fgets(input, MAXSCRITTORE, stdin) == NULL) {
            continue;
        }

        // Rimuove i caratteri di nuova linea (\n o \r\n) se presenti
        input[strcspn(input, "\r\n")] = 0;


        if (strcmp(input, "0") == 0 || strcmp(input, "1") == 0 || strcmp(input, "2") == 0 || strcmp(input, "3") == 0 || strcmp(input, "4") == 0 || strcmp(input, "5") == 0 || strcmp(input, "6") == 0 || strcmp(input, "7") == 0 || strcmp(input, "8") == 0 || strcmp(input, "9") == 0 
           || strcmp(input, "Q") == 0 || strcmp(input, "q") == 0) {
            return 1; // Input valido
        }
        printf("\nFormato non valido! Usa: numero tra 0 e 9 oppure 'q' per tornare al menu\n");
        return 0;

    }
}

// ==========================================================
//  FUNZIONE PER L'ENTRATA IN PARTITA
// ==========================================================

void funzione_entra_partita(){
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];


    while (1)
    {
         // Ricevi messaggio dal server
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }


        if (strcmp(buffer, MSG_NO_GAME)==0) {
            // Se il server ha restituito "MSG_NO_GAME", significa che non ci sono partite disponibili
            printf("Nessuna partita disponibile. Torna al menu principale.\n");
            return;
        }else{
            // Mostra il messaggio ricevuto
            CLEAR_SCREEN();
            printf("%s", buffer);
        }

        // Invia la mossa
        while (1) {
            memset(input, 0, MAXSCRITTORE);  // Fixed buffer size
            if (get_valid_match(input)) {
                break; // Manda l'input valido e esce dal loop
            }
        }


        if(strstr(input, "q") || strstr(input, "Q")){
            // Invia richiesta di uscita
            send(sd, MSG_CLIENT_QUIT, strlen(MSG_CLIENT_QUIT), 0);
            return;
        }else{
            // Invia la scelta al server
            send(sd, input, strlen(input), 0);
        }

        // Ricevi messaggio dal server
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }


        if(strcmp(buffer, MSG_JOIN_ERROR)==0) {
            // Se il server ha restituito un errore, significa che la partita è piena o non esistente
            printf("Partita piena o non esistente. Torna al menu principale.\n");            
            return;
        }

        if(strstr(buffer, MSG_SERVER_START)) {

            gioca_partita(AVVERSARIO);
            return;  // Fixed function name
        }
    }
    

}


// ==========================================================
//  FUNZIONE PER OTTENERE LA MOSSA DEL CLIENT DURANTE LA PARTITA
// ==========================================================



// Funzione per ottenere la mossa del client
int get_valid_move(char *input) {
    // Inizializza il buffer 
    memset(input, 0, MAXSCRITTORE);  // Fixed buffer size
    printf("Inserisci la tua mossa (1-9)-> ");
    // Legge l'input dell'utente
    // Se fgets restituisce NULL, significa che c'è stato un errore o EOF
    // In tal caso, continua il ciclo per chiedere di nuovo l'input
    if (fgets(input, MAXSCRITTORE, stdin) == NULL) {
        return 0; // Input non valido
    }
    // Rimuove i caratteri di nuova linea (\n o \r\n) se presenti
    input[strcspn(input, "\r\n")] = 0;
    // Controlla se l'input è un numero valido tra 1 e 9
    if (isdigit(input[0]) && strlen(input) == 1 && input[0] >= '1' && input[0] <= '9') {
        return 1; // Input valido
    } else {
        printf("Formato non valido! Usa: numero tra 1 e 9\n");
        return 0; // Input non valido
    }
}

// ==========================================================
//  MENU PER RICHIEDERE UNA RIVINCITA (SOLO IN CASO DI VITTORIA O SCONFITTA)
// ==========================================================

void play_again_menu() {
    char input[MAXSCRITTORE];

    while (1)
    {
        CLEAR_SCREEN();
        printf("\n=== Partita Terminata ===\n");
            printf("1. Gioca ancora\n");
            printf("2. Esci\n");
            printf("========================\n");
        printf("Scegli un'opzione: ");
        // Inizializza il buffer
        memset(input, 0, MAXSCRITTORE);  // Fixed buffer size
        if (fgets(input, MAXSCRITTORE, stdin) == NULL) {
            printf("Errore nella lettura dell'input.\n");
            return; // Input non valido
        }
    
        // Rimuove i caratteri di nuova linea (\n o \r\n) se presenti
        input[strcspn(input, "\r\n")] = 0;
    
        //Controlla se l'input è valido
        if (strcmp(input, "1") == 0) {
            // Invia richiesta di rivincita
            send(sd , MSG_CLIENT_REMATCH, strlen(MSG_CLIENT_REMATCH), 0);
            // Inizia una nuova partita
            CLEAR_SCREEN();
            printf("Hai scelto di giocare ancora!\n");
            printf("Aspettando che entri un nuovo avversario...\n");
            gioca_partita(PROPRIETARIO);
            break;
        } else if (strcmp(input, "2") == 0) {
            //torna al menu principale
            send(sd, MSG_CLIENT_QUIT, strlen(MSG_CLIENT_QUIT), 0);
            //ritorna al menu principale
            //controll che ricevo MSG_SERVER_MENU
            memset(input, 0, MAXSCRITTORE);
            if (recv(sd, input, MAXSCRITTORE, 0) <= 0) {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }
            //Controllo che il messaggio ricevuto sia quello giusto
            if (strcmp(input, MSG_SERVER_MENU) != 0) {
                printf("Errore nel ritorno al menu principale.\n");
                exit(EXIT_FAILURE);
            }
            // Mostra il messaggio ricevuto
            funzione_menu();
            break;
        } else {
            printf("Scelta non valida. Riprova...\n");
            usleep(100000); // Aspetta 1 secondo prima di ripetere il menu
            continue;
        }
    }
    

    
}

// ==========================================================
//  MENU PER RICHIEDERE UNA RIVINCITA (SOLO IN CASO DI PAREGGIO)
// ==========================================================


void play_again_menu_draw() {
    char input[MAXSCRITTORE];

    while (1)
    {
        CLEAR_SCREEN();
        printf("\n=== Partita Terminata Draw ===\n");
            printf("1. Gioca ancora\n");
            printf("2. Esci\n");
            printf("========================\n");
        printf("Scegli un'opzione: ");
        // Inizializza il buffer
        memset(input, 0, MAXSCRITTORE);  // Fixed buffer size
        if (fgets(input, MAXSCRITTORE, stdin) == NULL) {
            printf("Errore nella lettura dell'input.\n");
            return; // Input non valido
        }
    
        // Rimuove i caratteri di nuova linea (\n o \r\n) se presenti
        input[strcspn(input, "\r\n")] = 0;
    
        //Controlla se l'input è valido
        if (strcmp(input, "1") == 0) {
            // Invia richiesta di rivincita
            send(sd , MSG_CLIENT_REMATCH, strlen(MSG_CLIENT_REMATCH), 0);
            // Inizia una nuova partita
            break;
        } else if (strcmp(input, "2") == 0) {
            //torna al menu principale
            send(sd, MSG_CLIENT_QUIT, strlen(MSG_CLIENT_QUIT), 0);
            //ritorna al menu principale
            //controll che ricevo MSG_SERVER_MENU
            memset(input, 0, MAXSCRITTORE);
            if (recv(sd, input, MAXSCRITTORE, 0) <= 0) {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }
            //Controllo che il messaggio ricevuto sia quello giusto
            if (strcmp(input, MSG_SERVER_MENU) != 0) {
                printf("Errore nel ritorno al menu principale.\n");
                exit(EXIT_FAILURE);
            }
            // Mostra il messaggio ricevuto
            funzione_menu();
            break;
        } else {
            printf("Scelta non valida. Riprova...\n");
            usleep(100000); // Aspetta 1 secondo prima di ripetere il menu
            continue;
        }
    }
    
}

// ==========================================================
// FUNZIONE PER GESTIRE LA PARTITA
// ==========================================================

void gioca_partita(const enum tipo_giocatore tipo_giocatore) {
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];

    // Invia la mossa
    while (1) {

    //Questa struttura gestisce i messaggi in arrivo dal server, gestendo la partita in modo differente tra Proprietario e Avversario
    // Ricevi messaggio dal server
    memset(buffer, 0, MAXLETTORE);
    if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
        printf("Connessione al server persa.\n");
        exit(EXIT_FAILURE);
    }


    if (strcmp(buffer, MSG_SERVER_BOARD) == 0) {
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }
        // Mostra la board iniziale
        CLEAR_SCREEN();
        printf("%s", buffer);
    } 
    
    if (strcmp(buffer, MSG_YOUR_TURN) == 0) {

        // Invia la mossa
        while (1) {
            memset(input, 0, MAXSCRITTORE);  // Fixed buffer size
            if (get_valid_move(input)) {
                // Invia la scelta al server
                send(sd, input, strlen(input), 0);
                break; // Manda l'input valido e esce dal loop
            }
        }
    }

    if (strcmp(buffer, MSG_INVALID_MOVE) == 0) {
        // Mostra il messaggio ricevuto
        printf("Hai inserito una mossa non valida. Riprova.\n");
        // Invia la mossa
        while (1) {
            memset(input, 0, MAXSCRITTORE);  // Fixed buffer size
            if (get_valid_move(input)) {
                // Invia la scelta al server
                send(sd, input, strlen(input), 0);
                break; // Manda l'input valido e esce dal loop
            }
        }
    }

    
    
    if (strcmp(buffer, MSG_OPPONENT_TURN) == 0) {
        printf("Aspetta il turno dell'avversario...\n");
        continue;
    }

    
    if (strcmp(buffer, MSG_SERVER_WIN)==0) {

        printf("Hai vinto!\n");
        // Mostra il messaggio ricevuto
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }


        //Controllo che il messaggio ricevuto sia quello giusto
        if (strcmp(buffer, MSG_SERVER_REMATCH) == 0) {
                play_again_menu();
            
        }

    } else if (strcmp(buffer, MSG_SERVER_LOSE)==0) {
        // Mostra il messaggio ricevuto


        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
        printf("Connessione al server persa.\n");
        exit(EXIT_FAILURE);
        }
        if (strcmp(buffer, MSG_SERVER_MENU) == 0) {
            funzione_menu();
            return;
        }
    } else if (strcmp(buffer, MSG_SERVER_DRAW)==0) {
        // Mostra il messaggio ricevuto

        if(tipo_giocatore == PROPRIETARIO) {
            //Controllo se ricevo MSG_SERVER_REMATCH
            memset(buffer, 0, MAXLETTORE);
            if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }
            //Controllo che il messaggio ricevuto sia quello giusto
            if (strcmp(buffer, MSG_SERVER_REMATCH) == 0) {
                // Mostra il messaggio ricevuto
                
                // Invia la risposta al server
                play_again_menu_draw();
                printf("Aspettando scelta del guest... \n");
                memset(buffer, 0, MAXLETTORE);
                if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
                    printf("Connessione al server persa.\n");
                    exit(EXIT_FAILURE);
                }
                //Controllo che il messaggio ricevuto sia quello giusto
                if (strcmp(buffer, MSG_SERVER_ADMIN_QUIT) == 0) {
                    
                    printf("Il Guest ha abbandonato la partita.\n");
                    printf("Tornerai al menu principale\n");
                    sleep(1);
                    return;      
                } else if (strcmp(buffer, MSG_SERVER_REMATCH) == 0) {
                    // Mostra il messaggio ricevuto
                    
                    // Invia la risposta al server
                    gioca_partita(PROPRIETARIO);
                }
            }
        } else {
            printf("Hai pareggiato!\n");
            printf("Aspettando scelta del proprietario della partita... \n");
            //Controllo se ricevo MSG_SERVER_REMATCH
            memset(buffer, 0, MAXLETTORE);
            if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }
            //Controllo che il messaggio ricevuto sia quello giusto
            if (strcmp(buffer, MSG_WAITING_REMATCH) == 0) {
                memset(buffer, 0, MAXLETTORE);
                if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
                    printf("Connessione al server persa.\n");
                    exit(EXIT_FAILURE);
                }
                //Controllo che il messaggio ricevuto sia quello giusto
                if (strcmp(buffer, MSG_SERVER_REMATCH) == 0) {
                    // Mostra il messaggio ricevuto
                    
                    // Invia la risposta al server
                    play_again_menu_draw();
                    gioca_partita(AVVERSARIO);
                } else if (strcmp(buffer, MSG_SERVER_ADMIN_QUIT) == 0) {
                    
                    printf("Il proprietario ha abbandonato la partita.\n");
                    printf("Tornerai al menu principale\n");
                    sleep(1);
                    return;      
                }
            }
        }
    } else if (strcmp(buffer, MSG_SERVER_OPPONENT_LEFT)==0) {
        // Mostra il messaggio ricevuto
        
        printf("L'avversario si è disconnesso... Hai vinto!\n");
        sleep(1);
        play_again_menu();
    }
  }
}




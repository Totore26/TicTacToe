#include "funzioni.h"
#include "strutture.h"
#include <sys/time.h>



/*  FUNZIONI PER CONNESSIONE CON IL SERVER  */

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



void gestisci_errore()
{
    if (errno == EAGAIN || errno == EWOULDBLOCK) printf("disconnesso per inattività\n");
    else 
    {   
        printf("Si è verificato un errore\n");
        close(sd);
    }
    exit(EXIT_FAILURE);
}

/************************************************************************************************************************************************************************* */
/*  FUNZIONI GESTIONE INPUT  */


// Funzione per ottenere la mossa del client
int get_valid_move(char *input) {
    int row, col;
    while (1) {
        printf("\nInserisci la tua mossa (riga,colonna) [0-2]: ");

        if (fgets(input, MAXSCRITTORE, stdin) == NULL) {
            continue;
        }

        // Rimuove i caratteri di nuova linea (\n o \r\n) se presenti
        input[strcspn(input, "\r\n")] = 0;

        // Se l'input è vuoto, invia "1" al server per chiedere il tabellone
        if (strlen(input) == 0) {
            strcpy(input, "1"); // Imposta "1" per il tabellone
            return 0; // Ritorna 0 per segnalare che è stato premuto INVIO
        }

        // Se il formato non è valido (non è riga,colonna), chiedi di nuovo
        if (sscanf(input, "%d,%d", &row, &col) != 2) {
            printf("\nFormato non valido! Usa: numero,numero\n");
            continue;
        }

        if (row < 0 || row > 2 || col < 0 || col > 2) {
            printf("\nPosizione non valida! Usa numeri tra 0 e 2\n");
            continue;
        }

        return 1; // Input valido
    }
}



// Funzione per ottenere la mossa del client
int get_valid_match(char *input) {
    while (1) {
        //printf("\nInserisci la tua mossa (riga,colonna) [0-2]: ");
        
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

void play_again_menu(int socket_fd, int is_winner) {
    CLEAR_SCREEN();
    printf("\n=== Partita Terminata ===\n");
    if (is_winner) {
        printf("1. Gioca ancora\n");
        printf("2. Esci\n");
    } else {
        printf("Hai perso. Tornerai al menu principale.\n");
        send(socket_fd, "2", 1, 0);  // Invia direttamente "2" per uscire
        return;
    }

    int choice = get_valid_menu_choice(1, 2);
    char choice_str[2];
    sprintf(choice_str, "%d", choice);
    send(socket_fd, choice_str, strlen(choice_str), 0);

    // Attendi risposta dal server
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(socket_fd, buffer, BUFFER_SIZE - 1, 0) <= 0) {
        printf("Connessione al server persa.\n");
        exit(EXIT_FAILURE);
    }

    if (strstr(buffer, "wait")) {
        printf("In attesa dell'avversario...\n");
        return;
    }

    if (strstr(buffer, "exit")) {
        exit(EXIT_SUCCESS);
    }
    // Altrimenti continua con il gioco
}


void funzione_menu() {
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];
    
    while (1) {
        // Ricevi messaggio dal server
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }

        CLEAR_SCREEN();
        printf("%s", buffer);

        fgets(input, MAXLETTORE, stdin);
        if (strstr(input, "1")) {
            // Invia richiesta di creazione partita
            send(sd, MSG_CLIENT_CREAATE, strlen(MSG_CLIENT_CREAATE), 0);
            funzione_crea_partita();
            break;
        } else if (strstr(input, "2")) {
            // Invia richiesta di unione a partita
            send(sd, MSG_CLIENT_JOIN, strlen(MSG_CLIENT_JOIN), 0);
            funzione_entra_partita();
            break;
        } else if (strstr(input, "3")) {
            // Invia richiesta di uscita
            send(sd, MSG_CLIENT_QUIT, strlen(MSG_CLIENT_QUIT), 0);
            close(sd);
            exit(EXIT_SUCCESS);
            } else {
                printf("Scelta non valida. Riprova.\n");
            }
        } // Close the while loop

}

void funzione_entra_partita(){
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];

    // Ricevi messaggio dal server
    memset(buffer, 0, MAXLETTORE);
    if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
        printf("Connessione al server persa.\n");
        exit(EXIT_FAILURE);
    }

    if (strstr(buffer, "0")) {
        // Se il server ha restituito "0", significa che non ci sono partite disponibili
        printf("Nessuna partita disponibile. Torna al menu principale.\n");
        return;
    }else{
        // Mostra il messaggio ricevuto
        printf("%s", buffer);
    }

    // Invia la mossa
    while (1) {
        memset(input, 0, MAXLETTORE);
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

    if(strstr(buffer, MSG_SERVER_PARTITA_PIENA) || strstr(buffer, MSG_SERVER_PARTITA_NON_ESISTENTE)){
        printf("Partita piena o non esistente. Torna al menu principale.\n");
        return;
    }

    if(strstr(buffer, MSG_SERVER_START)) {
        gioca_partite(AVVERSARIO);
    }
}

void funzione_crea_partita(){
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];

    // Ricevi messaggio dal server
    memset(buffer, 0, MAXLETTORE);
    if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
        printf("Connessione al server persa.\n");
        exit(EXIT_FAILURE);
    }

    // Mostra il messaggio ricevuto, inserire nome partita
    printf("%s", buffer);

    fgets(input, MAXLETTORE, stdin);
    send(sd, input, strlen(input), 0);



    while (1)
    {
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }

        if (strstr(buffer, MSG_SERVER_START)){
            gioca_partite(PROPRIETARIO);
        }
    }
    

}


void gioca_partita(const enum tipo_giocatore tipo_giocatore) {
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];

    if (tipo_giocatore ==  PROPRIETARIO){

    }else{

    }

}


#include "funzioni.h"
#include "strutture.h"
#define MAX_NOTIFICHE 5 // Numero massimo di notifiche da mostrare
#define MAX_LUNGHEZZA_NOTIFICA 30 // Lunghezza massima per ogni notifica
char notifiche[MAX_NOTIFICHE][128]; // Array per memorizzare le notifiche
int indiceNotifica = 0; // Indice per la prossima notifica
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
    timer.tv_sec = 300; // Timer di 300 secondi
    timer.tv_usec = 0;

    // timer per invio e ricezione
    if (setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &timer, sizeof(timer)) < 0 || setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer)) < 0)
        perror("set socket timer error"), exit(EXIT_FAILURE);

    // disattiva bufferizzazione socket
    if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0)
        perror("tcp nodelay error"), exit(EXIT_FAILURE);

    memset(&ser_add, 0, sizeof(ser_add));
    ser_add.sin_family = AF_INET;
    ser_add.sin_port = htons(8080);
    ser_add.sin_addr.s_addr = inet_addr("127.0.0.1");

    // non c'è bisogno di inizializzare manualmente la porte client e fare il bind

    if (connect(sd, (struct sockaddr *)&ser_add, lenght) < 0)
        perror("connect error"), exit(EXIT_FAILURE);
}

// Funzione ausiliaria per gestire il segnale SIGTERM
// Questa funzione viene chiamata quando il programma riceve un segnale SIGTERM
// Chiude il socket e termina il programma
// Utile per Docker
void SIGTERM_handler()
{
    close(sd);
    exit(EXIT_SUCCESS);
}

// ==========================================================
//  FUNZIONE PER GESTIRE IL MENU
// ==========================================================

// Funzione Ausiliaria
//  Funzione per attendere l'input dell'utente prima di continuare
//  Questa funzione aspetta che l'utente prema Invio per continuare
void attendo_invio()
{
    printf("Premi Invio per continuare...\n");
    fflush(stdout);
    while (getchar() != '\n')
        ; // blocca fino a Invio
}

void aggiungi_notifica(const char *messaggio) {
    snprintf(notifiche[indiceNotifica], sizeof(notifiche[indiceNotifica]), "%s", messaggio);
    indiceNotifica = (indiceNotifica + 1) % MAX_NOTIFICHE; // Incrementa l'indice in modo circolare
}

void gestisci_notifica_server(const char *buffer) {
    char messaggio[128];

    if (strncmp(buffer, MSG_NEW_USER_REGISTERED, strlen(MSG_NEW_USER_REGISTERED)) == 0) {
        char *nome = strchr(buffer, ':');
        if (nome) nome++;
        else nome = "Un nuovo utente";
        snprintf(messaggio, sizeof(messaggio), "🔔 Nuovo utente registrato: %s", nome);
        aggiungi_notifica(messaggio);
    } 
    else if (strncmp(buffer, MSG_USER_DISCONNECTED, strlen(MSG_USER_DISCONNECTED)) == 0) {
        char *nome = strchr(buffer, ':');
        if (nome) nome++;
        else nome = "Un utente";
        snprintf(messaggio, sizeof(messaggio), "🚪 Utente disconnesso: %s", nome);
        aggiungi_notifica(messaggio);
    } 
    else if (strncmp(buffer, MSG_NEW_GAME_CREATED, strlen(MSG_NEW_GAME_CREATED)) == 0) {
        char *nomeAdmin = strchr(buffer, ':');
        if (nomeAdmin) nomeAdmin++;
        else nomeAdmin = "Sconosciuto";
        snprintf(messaggio, sizeof(messaggio), "🎮 Nuova partita creata da %s", nomeAdmin);
        aggiungi_notifica(messaggio);
    }
    else if (strcmp(buffer, MSG_SERVER_MENU) == 0) {
        return;
    }

    printf(" Scegli un'opzione-> ");
    fflush(stdout);
}

void funzione_menu() {
    char input[MAXSCRITTORE];
    char buffer[MAXLETTORE];
    CLEAR_SCREEN();

    while (1) {
        CLEAR_SCREEN();

        // Stampa le notifiche come elenco semplice
        printf("\nNOTIFICHE RECENTI:\n");
        for (int i = 0; i < MAX_NOTIFICHE; i++) {
            int index = (indiceNotifica + i) % MAX_NOTIFICHE;
            if (strlen(notifiche[index]) > 0) {
                printf("- %s\n", notifiche[index]);
            }
        }
        printf("\n");
        // Stampa il menu principale
        printf("\n");
        printf("╔════════════════════════════════╗\n");
        printf("║        MENU PRINCIPALE         ║\n");
        printf("╠════════════════════════════════╣\n");
        printf("║  1. Crea partita               ║\n");
        printf("║  2. Unisciti a partita         ║\n");
        printf("║  3. Esci                       ║\n");
        printf("╚════════════════════════════════╝\n");
        printf(" Scegli un'opzione-> ");


        fflush(stdout);

        // Usa select per ascoltare sia stdin che il socket
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sd, &readfds);
        int maxfd = sd > STDIN_FILENO ? sd : STDIN_FILENO;

        int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ready < 0) {
            perror("select error");
            continue;
        }

        // Notifica dal server
        if (FD_ISSET(sd, &readfds)) {
            memset(buffer, 0, MAXLETTORE);
            ssize_t n = recv(sd, buffer, MAXLETTORE, 0);
            if (n <= 0) {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }
            gestisci_notifica_server(buffer);
            continue;
        }

        // Input utente
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            memset(input, 0, MAXSCRITTORE);
            if (fgets(input, MAXSCRITTORE, stdin) == NULL) {
                continue;
            }
            input[strcspn(input, "\n")] = 0;

            if (strcmp(input, "1") == 0) {
                send(sd, MSG_CLIENT_CREAATE, strlen(MSG_CLIENT_CREAATE), 0);
                funzione_crea_partita();
                break;
            } else if (strcmp(input, "2") == 0) {
                send(sd, MSG_CLIENT_JOIN, strlen(MSG_CLIENT_JOIN), 0);
                funzione_entra_partita();
                break;
            } else if (strcmp(input, "3") == 0) {
                send(sd, MSG_CLIENT_QUIT, strlen(MSG_CLIENT_QUIT), 0);
                close(sd);
                exit(EXIT_SUCCESS);
            } else {
                printf("Scelta non valida. Riprova...\n");
                usleep(800000);
            }
        }
    }
}
// ==========================================================
//  FUNZIONE PER GESTIRE LA CREAZIONE DELLA PARTITA
// ==========================================================

void funzione_crea_partita()
{
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];

    // Ricevi messaggio dal server
    memset(buffer, 0, MAXLETTORE);
    if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
    {
        printf("Connessione al server persa.\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(buffer, MSG_SERVER_MAX_GAMES) == 0)
    {
        // Se il server ha restituito "MSG_SERVER_MAX_GAMES", significa che sono state raggiunte le partite massime
        printf("Massimo numero di partite raggiunto. Torna al menu principale.\n");
        return;
    }

    if (strcmp(buffer, MSG_WAITING_PLAYER) == 0)
    {
        // Se il server ha restituito "MSG_WAITING_PLAYER", significa che la partita è in attesa di un avversario

        while (1)
        {

            CLEAR_SCREEN();
            printf("Aspettando un avversario...\n");

            memset(buffer, 0, MAXLETTORE);
            if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
            {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }

            if (strcmp(buffer, MSG_SERVER_JOIN_REQUEST) == 0)
            {
                // Se il server ha restituito "MSG_SERVER_JOIN_REQUEST", significa che un avversario vuole unirsi alla partita

                // Gestione della richiesta di un avversario
                while (1)
                {
                    CLEAR_SCREEN();
                    printf("Un avversario vuole unirsi alla tua partita!\n");
                    printf("Vuoi accettare? [s/n]: ");

                    memset(input, 0, MAXSCRITTORE);
                    if (fgets(input, MAXSCRITTORE, stdin) == NULL)
                    {
                        continue;
                    }

                    // Rimuove il newline
                    input[strcspn(input, "\n")] = 0;

                    if (strcmp(input, "s") == 0 || strcmp(input, "S") == 0)
                    {
                        // Invia risposta di accettazione
                        printf("Hai accettato l'avversario!\n");
                        usleep(500000);
                        send(sd, MSG_CLIENT_ACCEPT, strlen(MSG_CLIENT_ACCEPT), 0);
                        break;
                    }
                    else if (strcmp(input, "n") == 0 || strcmp(input, "N") == 0)
                    {
                        // Invia risposta di rifiuto
                        printf("Hai rifiutato l'avversario!\n");
                        usleep(500000);
                        send(sd, MSG_CLIENT_REFUSE, strlen(MSG_CLIENT_REFUSE), 0);
                        break;
                    }
                    else
                    {
                        printf("Scelta non valida. Riprova...\n");
                        usleep(800000);
                        continue;
                    }
                }
            }

            // Una volta accettato l'avversario, aspetto che il server MSG_SERVER_START per iniziare una partita
            if (strcmp(buffer, MSG_SERVER_START) == 0)
            {
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
int get_valid_match(char *input)
{
    while (1)
    {
        printf("\nScegli una partita disponibile (0-9) \nOppure premi 'q' per tornare al menù principale:\n");
        // Legge l'input dell'utente
        // Se fgets restituisce NULL, significa che c'è stato un errore o EOF
        // In tal caso, continua il ciclo per chiedere di nuovo l'input
        if (fgets(input, MAXSCRITTORE, stdin) == NULL)
        {
            continue;
        }

        // Rimuove i caratteri di nuova linea (\n o \r\n) se presenti
        input[strcspn(input, "\r\n")] = 0;

        if (strcmp(input, "0") == 0 || strcmp(input, "1") == 0 || strcmp(input, "2") == 0 || strcmp(input, "3") == 0 || strcmp(input, "4") == 0 || strcmp(input, "5") == 0 || strcmp(input, "6") == 0 || strcmp(input, "7") == 0 || strcmp(input, "8") == 0 || strcmp(input, "9") == 0 || strcmp(input, "Q") == 0 || strcmp(input, "q") == 0)
        {
            return 1; // Input valido
        }
        printf("\nFormato non valido! Usa: numero tra 0 e 9 oppure 'q' per tornare al menu\n");
        return 0;
    }
}

// ==========================================================
//  FUNZIONE PER L'ENTRATA IN PARTITA
// ==========================================================

void funzione_entra_partita()
{
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];

    while (1)
    {
        // Ricevi messaggio dal server
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
        {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }

        if (strcmp(buffer, MSG_NO_GAME) == 0)
        {
            // Se il server ha restituito "MSG_NO_GAME", significa che non ci sono partite disponibili
            CLEAR_SCREEN();
            printf("Nessuna partita disponibile. Torna al menu principale.\n");
            attendo_invio();
            return;
        }
        else
        {
            // Se c'è almeno una lobby allora mostra il messaggio ricevuto
            CLEAR_SCREEN();
            printf("%s", buffer);
            printf("Scegli una partita o premi q per tornare al menu:\n");
        }

        while (1) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);  // input utente
            FD_SET(sd, &readfds);           // messaggi dal server
    
            int maxfd = sd > STDIN_FILENO ? sd : STDIN_FILENO;
    
            int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
            if (ready < 0) {
                perror("Errore nella funzione select");
                continue; // Continua il ciclo in caso di errore
            }
    
            // 📥 Se arriva un messaggio dal server vedo se si tratta nella macro 
            if (FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, MAXLETTORE);
                ssize_t bytes_received = recv(sd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_received <= 0) {
                    printf("Connessione chiusa dal server.\n");
                    break; // Esce dal ciclo in caso di errore o chiusura
                }
    
                if (strcmp(buffer, MSG_SERVER_GAME_LIST) == 0) {  // Usa la macro, non la stringa letterale
                    // Aspetta il secondo messaggio: la lista aggiornata
                    memset(buffer, 0, MAXLETTORE);
                    bytes_received = recv(sd, buffer, sizeof(buffer) - 1, 0);
                    if (bytes_received <= 0) {
                        printf("Connessione chiusa dal server.\n");
                        break; // Esce dal ciclo in caso di errore o chiusura
                    }
                    // stampo la lista aggiornata
                    CLEAR_SCREEN();
                    printf("%s", buffer);
                    printf("Scegli una partita o premi q per tornare al menu:\n");
                } 
            }
    
            // 👤 Input utente disponibile
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                memset(input, 0, MAXSCRITTORE);
                if (get_valid_match(input)) {
                    break; // input valido → esce dal ciclo
                }
            }
        }

        // Controlla se l'utente vuole tornare al menu principale
        if (strstr(input, "q") || strstr(input, "Q"))
        {
            // Invia richiesta di uscita
            send(sd, MSG_CLIENT_QUIT, strlen(MSG_CLIENT_QUIT), 0);
            return;
        }
        else
        {
            // Invia la scelta della partita al server
            send(sd, input, strlen(input), 0);
        }

        CLEAR_SCREEN();
        printf("Hai scelto la partita con ID: %s\n", input);
        printf("Recupero informazioni in corso...\n");

        // Ricevi messaggio dal server
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
        {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }

        if (strcmp(buffer, MSG_JOIN_ERROR) == 0)
        {
            // Se il server ha restituito un errore, significa che la partita è piena o non esistente
            usleep(800000);
            printf("Partita piena o non esistente. Torna al menu principale.\n");
            attendo_invio();
            return;
        }
        else
        {
            // Se ricevo MSG_SERVER_REFUSE vuol dire che l'admin della partita ha rifiutato la mia richiesta
            if (strcmp(buffer, MSG_SERVER_REFUSE) == 0)
            {
                printf("L'admin della partita ha rifiutato la tua richiesta. Torna al menu principale.\n");
                attendo_invio();
                return;
            }

            //Se ricevo MSG_SERVER_ADMIN_QUIT vuol dire che l'admin della partita ha abbandonato la partita
            if (strcmp(buffer, MSG_SERVER_ADMIN_QUIT) == 0)
            {
                printf("L'admin della partita ha abbandonato. Torna al menu principale.\n");
                attendo_invio();
                return;
            }

            // Se ricevo MSG_SERVER_START vuol dire che la partita è iniziata
            if (strcmp(buffer, MSG_SERVER_START) == 0)
            {
                gioca_partita(AVVERSARIO);
                return;
            }
        }
    }
}

// ==========================================================
//  FUNZIONE PER OTTENERE LA MOSSA DEL CLIENT DURANTE LA PARTITA
// ==========================================================

// Funzione per ottenere la mossa del client
int get_valid_move(char *input)
{
    // Inizializza il buffer
    memset(input, 0, MAXSCRITTORE); 
    printf("Inserisci la tua mossa (1-9)-> ");
    // Legge l'input dell'utente
    // Se fgets restituisce NULL, significa che c'è stato un errore o EOF
    // In tal caso, continua il ciclo per chiedere di nuovo l'input
    if (fgets(input, MAXSCRITTORE, stdin) == NULL)
    {
        return 0; // Input non valido
    }
    // Rimuove i caratteri di nuova linea (\n o \r\n) se presenti
    input[strcspn(input, "\r\n")] = 0;
    // Controlla se l'input è un numero valido tra 1 e 9
    if (isdigit(input[0]) && strlen(input) == 1 && input[0] >= '1' && input[0] <= '9')
    {
        return 1; // Input valido
    }
    else
    {
        printf("Formato non valido! Usa: numero tra 1 e 9\n");
        return 0; // Input non valido
    }
}

// ==========================================================
//  MENU PER RICHIEDERE UNA RIVINCITA (SOLO IN CASO DI VITTORIA O SCONFITTA)
// ==========================================================

void play_again_menu()
{
    char input[MAXSCRITTORE];
    char buffer[MAXLETTORE];

    while (1)
    {
        CLEAR_SCREEN();
        printf("\n");
        printf("╔════════════════════════════════╗\n");
        printf("║       PARTITA TERMINATA        ║\n");
        printf("╠════════════════════════════════╣\n");
        printf("║  1. Gioca ancora               ║\n");
        printf("║  2. Esci                       ║\n");
        printf("╚════════════════════════════════╝\n");
        printf(" Scegli un'opzione-> ");
        // Inizializza il buffer
        memset(input, 0, MAXSCRITTORE); // Fixed buffer size
        if (fgets(input, MAXSCRITTORE, stdin) == NULL)
        {
            printf("Errore nella lettura dell'input.\n");
            return; // Input non valido
        }

        // Rimuove i caratteri di nuova linea (\n o \r\n) se presenti
        input[strcspn(input, "\r\n")] = 0;

        // Controlla se l'utente ha scelto di giocare ancora
        if (strcmp(input, "1") == 0)
        {
            // Invia richiesta di rivincita
            send(sd, MSG_CLIENT_REMATCH, strlen(MSG_CLIENT_REMATCH), 0);

            while (1)
            {
                CLEAR_SCREEN();
                printf("Hai scelto di giocare ancora!\n");
                printf("Aspettando che entri un nuovo avversario...\n");

                memset(buffer, 0, MAXLETTORE);
                if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
                {
                    printf("Connessione al server persa.\n");
                    exit(EXIT_FAILURE);
                }

                //Ciclo per gestire la richiesta di un avversario
                if (strcmp(buffer, MSG_SERVER_JOIN_REQUEST) == 0)
                {

                    while (1)
                    {
                        // Se il server ha restituito "MSG_SERVER_JOIN_REQUEST", significa che un avversario vuole unirsi alla partita
                        // Mostra il messaggio ricevuto
                        CLEAR_SCREEN();
                        printf("Un avversario vuole unirsi alla tua partita!\n");
                        printf("Vuoi accettare? [s/n]: ");
                        // Inizializza il buffer

                        memset(input, 0, MAXSCRITTORE); // Fixed buffer size
                        if (fgets(input, MAXSCRITTORE, stdin) == NULL)
                        { // Fixed buffer size
                            continue;
                        }

                        // Rimuove il newline
                        input[strcspn(input, "\n")] = 0;

                        if (strcmp(input, "s") == 0 || strcmp(input, "S") == 0)
                        {
                            // Invia risposta di accettazione
                            printf("Hai accettato l'avversario!\n");
                            usleep(500000);
                            send(sd, MSG_CLIENT_ACCEPT, strlen(MSG_CLIENT_ACCEPT), 0);
                            gioca_partita(PROPRIETARIO);
                            break;
                        }
                        else if (strcmp(input, "n") == 0 || strcmp(input, "N") == 0)
                        {
                            // Invia risposta di rifiuto
                            printf("Hai rifiutato l'avversario!\n");
                            usleep(500000);
                            send(sd, MSG_CLIENT_REFUSE, strlen(MSG_CLIENT_REFUSE), 0);
                            break;
                        }
                        else
                        {
                            printf("Scelta non valida. Riprova...\n");
                            usleep(800000);
                            continue;
                        }
                    }
                }
            }

            break;
        }
        // Controlla se l'utente ha scelto di uscire
        else if (strcmp(input, "2") == 0)
        {
            // torna al menu principale
            send(sd, MSG_CLIENT_QUIT, strlen(MSG_CLIENT_QUIT), 0);
            // ritorna al menu principale
            // controll che ricevo MSG_SERVER_MENU
            memset(input, 0, MAXSCRITTORE);
            if (recv(sd, input, MAXSCRITTORE, 0) <= 0)
            {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }
            // Controllo che il messaggio ricevuto sia quello giusto
            if (strcmp(input, MSG_SERVER_MENU) != 0)
            {
                printf("Errore nel ritorno al menu principale.\n");
                exit(EXIT_FAILURE);
            }
            // Mostra il messaggio ricevuto
            funzione_menu();
            break;
        }
        else
        {
            printf("Scelta non valida. Riprova...\n");
            attendo_invio();
            continue;
        }
    }
}

// ==========================================================
//  MENU PER RICHIEDERE UNA RIVINCITA (SOLO IN CASO DI PAREGGIO)
// ==========================================================

void play_again_menu_draw()
{
    char input[MAXSCRITTORE];

    while (1)
    {
        CLEAR_SCREEN();
        printf("\n");
        printf("╔════════════════════════════════╗\n");
        printf("║   PARTITA TERMINATA PAREGGIO   ║\n");
        printf("╠════════════════════════════════╣\n");
        printf("║  1. Gioca ancora               ║\n");
        printf("║  2. Esci                       ║\n");
        printf("╚════════════════════════════════╝\n");
        printf(" Scegli un'opzione-> ");
        // Inizializza il buffer
        memset(input, 0, MAXSCRITTORE); // Fixed buffer size
        if (fgets(input, MAXSCRITTORE, stdin) == NULL)
        {
            printf("Errore nella lettura dell'input.\n");
            return; // Input non valido
        }

        // Rimuove i caratteri di nuova linea (\n o \r\n) se presenti
        input[strcspn(input, "\r\n")] = 0;

        // Controlla se l'utente ha scelto di giocare ancora
        if (strcmp(input, "1") == 0)
        {
            // Invia richiesta di rivincita
            send(sd, MSG_CLIENT_REMATCH, strlen(MSG_CLIENT_REMATCH), 0);
            // Inizia una nuova partita
            break;
        }
        // Controlla se l'utente ha scelto di uscire
        else if (strcmp(input, "2") == 0)
        {
            // torna al menu principale
            send(sd, MSG_CLIENT_QUIT, strlen(MSG_CLIENT_QUIT), 0);
            // ritorna al menu principale
            // controll che ricevo MSG_SERVER_MENU
            memset(input, 0, MAXSCRITTORE);
            if (recv(sd, input, MAXSCRITTORE, 0) <= 0)
            {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }
            // Controllo che il messaggio ricevuto sia quello giusto
            if (strcmp(input, MSG_SERVER_MENU) != 0)
            {
                printf("Errore nel ritorno al menu principale.\n");
                exit(EXIT_FAILURE);
            }
            // Mostra il messaggio ricevuto
            funzione_menu();
            break;
        }
        else
        {
            printf("Scelta non valida. Riprova...\n");
            attendo_invio();
            continue;
        }
    }
}

// ==========================================================
// FUNZIONE PER GESTIRE LA PARTITA
// ==========================================================

void gioca_partita(const enum tipo_giocatore tipo_giocatore)
{
    char buffer[MAXLETTORE];
    char input[MAXSCRITTORE];

    // Invia la mossa
    while (1)
    {

        // Questa struttura gestisce i messaggi in arrivo dal server, gestendo la partita in modo differente tra Proprietario e Avversario
        //  Ricevi messaggio dal server
        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
        {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }

        if (strcmp(buffer, MSG_SERVER_BOARD) == 0)
        {
            memset(buffer, 0, MAXLETTORE);
            if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
            {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }
            // Mostra la board iniziale
            CLEAR_SCREEN();
            printf("%s", buffer);
        }

        if (strcmp(buffer, MSG_YOUR_TURN) == 0)
        {

            // Invia la mossa
            while (1)
            {
                memset(input, 0, MAXSCRITTORE); // Fixed buffer size
                if (get_valid_move(input))
                {
                    // Invia la scelta al server
                    send(sd, input, strlen(input), 0);
                    break; // Manda l'input valido e esce dal loop
                }
            }
        }

        if (strcmp(buffer, MSG_INVALID_MOVE) == 0)
        {
            // Mostra il messaggio ricevuto
            printf("Hai inserito una mossa non valida. Riprova.\n");
            // Invia la mossa
            while (1)
            {
                memset(input, 0, MAXSCRITTORE); // Fixed buffer size
                if (get_valid_move(input))
                {
                    // Invia la scelta al server
                    send(sd, input, strlen(input), 0);
                    break; // Manda l'input valido e esce dal loop
                }
            }
        }

        if (strcmp(buffer, MSG_OPPONENT_TURN) == 0)
        {
            printf("Aspetta il turno dell'avversario...\n");
            continue;
        }

        if (strcmp(buffer, MSG_SERVER_WIN) == 0)
        {

            printf("Hai vinto!\n");
            attendo_invio();
            // Mostra il messaggio ricevuto
            memset(buffer, 0, MAXLETTORE);
            if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
            {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }

            // Controllo che il messaggio ricevuto sia quello giusto
            if (strcmp(buffer, MSG_SERVER_REMATCH) == 0)
            {
                play_again_menu();
            }
        }
        else if (strcmp(buffer, MSG_SERVER_LOSE) == 0)
        {
            // Mostra il messaggio ricevuto
            printf("Hai perso!\n");
            attendo_invio();

            memset(buffer, 0, MAXLETTORE);
            if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
            {
                printf("Connessione al server persa.\n");
                exit(EXIT_FAILURE);
            }
            if (strcmp(buffer, MSG_SERVER_MENU) == 0)
            {
                funzione_menu();
                return;
            }
        }
        else if (strcmp(buffer, MSG_SERVER_DRAW) == 0)
        {
            // Mostra il messaggio ricevuto
            printf("Hai pareggiato!\n");

            if (tipo_giocatore == PROPRIETARIO)
            {
                attendo_invio();
                // Controllo se ricevo MSG_SERVER_REMATCH
                memset(buffer, 0, MAXLETTORE);
                if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
                {
                    printf("Connessione al server persa.\n");
                    exit(EXIT_FAILURE);
                }
                // Controllo che il messaggio ricevuto sia quello giusto
                if (strcmp(buffer, MSG_SERVER_REMATCH) == 0)
                {
                    // Mostra il messaggio ricevuto

                    // Invia la risposta al server
                    play_again_menu_draw();
                    printf("Aspettando scelta del guest... \n");
                    memset(buffer, 0, MAXLETTORE);
                    if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
                    {
                        printf("Connessione al server persa.\n");
                        exit(EXIT_FAILURE);
                    }
                    // Controllo che il messaggio ricevuto sia quello giusto
                    if (strcmp(buffer, MSG_SERVER_ADMIN_QUIT) == 0)
                    {

                        printf("Il Guest ha abbandonato la partita.\n");
                        printf("Tornerai al menu principale\n");
                        attendo_invio();
                        memset(buffer, 0, MAXLETTORE);
                        if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
                        {
                            printf("Connessione al server persa.\n");
                            exit(EXIT_FAILURE);
                        }
                        if (strcmp(buffer, MSG_SERVER_MENU) == 0)
                            funzione_menu();

                        return;
                    }
                    else if (strcmp(buffer, MSG_SERVER_REMATCH) == 0)
                    {
                        // Mostra il messaggio ricevuto

                        // Invia la risposta al server
                        gioca_partita(PROPRIETARIO);
                    }
                }
            }
            else
            {
                CLEAR_SCREEN();
                printf("Hai pareggiato!\n");
                printf("Aspettando scelta del proprietario della partita... \n");
                // Controllo se ricevo MSG_SERVER_REMATCH
                memset(buffer, 0, MAXLETTORE);
                if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
                {
                    printf("Connessione al server persa.\n");
                    exit(EXIT_FAILURE);
                }
                // Controllo che il messaggio ricevuto sia quello giusto
                if (strcmp(buffer, MSG_WAITING_REMATCH) == 0)
                {
                    memset(buffer, 0, MAXLETTORE);
                    if (recv(sd, buffer, MAXLETTORE, 0) <= 0)
                    {
                        printf("Connessione al server persa.\n");
                        exit(EXIT_FAILURE);
                    }
                    // Controllo che il messaggio ricevuto sia quello giusto
                    if (strcmp(buffer, MSG_SERVER_REMATCH) == 0)
                    {
                        // Mostra il messaggio ricevuto

                        // Invia la risposta al server
                        play_again_menu_draw();
                        gioca_partita(AVVERSARIO);
                    }
                    else if (strcmp(buffer, MSG_SERVER_ADMIN_QUIT) == 0)
                    {

                        printf("Il proprietario ha abbandonato la partita.\n");
                        printf("Tornerai al menu principale\n");
                        attendo_invio();
                        return;
                    }
                }
            }
        }
        else if (strcmp(buffer, MSG_SERVER_OPPONENT_LEFT) == 0)
        {
            // Mostra il messaggio ricevuto

            printf("L'avversario si è disconnesso... Hai vinto!\n");
            printf("Tornerai al menu principale\n");
            attendo_invio();
            return;
        }
    }
}

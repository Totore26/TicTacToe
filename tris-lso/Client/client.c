#include "funzioni.h"
#include "strutture.h"

int sd = 0;  // Socket del client

int main() {
    inizializza_socket();
    char buffer[1024];

    signal(SIGTERM, SIGTERM_handler);

    // Countdown per la connessione al server
    printf("Connessione al server in corso...\n");
    for (int i = 5; i >= 0; i--) {
        sleep(1);
        printf("%d\n", i);
    }

    char nome[50];
    // Chiedi all'utente di registrarsi con un nome
    printf("Inserisci il tuo nome: ");
        fgets(nome, sizeof(nome), stdin);
        nome[strcspn(nome, "\n")] = 0; // Rimuove il newline

        if (strlen(nome) > 0) {
            // Invia il nome al server
            send(sd, nome, strlen(nome), 0);

            // Ricevi conferma dal server
            memset(buffer, 0, sizeof(buffer));
            if (recv(sd, buffer, sizeof(buffer), 0) > 0) {
                printf("%s\n", buffer); // Stampa il messaggio di conferma
                sleep(3);
            } else {
                printf("Errore nella registrazione. Riprova.\n");
            }
        } else {
            printf("Il nome non può essere vuoto. Riprova.\n");
        }
    


    // Mostra il menu principale
    while (1) {
        // Inizializza il buffer
        memset(buffer, 0, sizeof(buffer));
        // Ricevi messaggio dal server
        if (recv(sd, buffer, sizeof(buffer), 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }

        // Controlla il messaggio ricevuto
        if (strcmp(buffer, MSG_SERVER_MENU) == 0) {
            funzione_menu();
        }
    }

    // Chiudi il socket
    close(sd);
    return 0;
}
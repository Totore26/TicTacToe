#include "funzioni.h"
#include "strutture.h"

int sd = 0;  // Socket del client

int main() {
    inizializza_socket();
    char buffer[1024];

    for (int i = 5; i >= 0; i--)
    {
        sleep(1);
        printf("%d\n",i);
    }

    while (1) {
        // Inizializza il buffer
        memset(buffer, 0, sizeof(buffer));
        // Ricevi messaggio dal server
        if (recv(sd, buffer, sizeof(buffer), 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }
        

        // Controlla il messaggio ricevuto
        if (strcmp(buffer, MSG_SERVER_MENU)==0) {
            funzione_menu();
        }

    }

    close(sd);
    return 0;
}

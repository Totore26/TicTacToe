#include "funzioni.h"
#include "strutture.h"

int sd = 0;  // Socket del client

int main() {
    inizializza_socket();
    char buffer[1024];

    //Ciclo di attesa per la connessione al server,  con un countdown di 5 secondi
    //Utile per Docker
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

    // Chiudi il socket
    close(sd);
    return 0;
}

#include "funzioni.h"
#include "strutture.h"

int sd = 0;  // Add proper type declaration

int main() {
    inizializza_socket();
    char buffer[1024];
    while (1) {
        printf("In attesa di messaggi dal server...\n");
        memset(buffer, 0, sizeof(buffer));
        if (recv(sd, buffer, sizeof(buffer), 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }
        printf("Messaggio ricevuto: %s\n", buffer);
        for (int i = 0; i < 5; i++)
        {
            printf("%d \n", i);
            sleep(1);
        }
        
        // Controlla il messaggio ricevuto
        if (strstr(buffer, MSG_SERVER_MENU)) {
            funzione_menu();
        }

    }

    close(sd);
    return 0;
}

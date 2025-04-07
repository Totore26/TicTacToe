#include "funzioni.h"
#include "strutture.h"

sd = 0;

int main() {

    inizializza_socket();

    char buffer[1024];
    while (1) {

        memset(buffer, 0, MAXLETTORE);
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) {
            printf("Connessione al server persa.\n");
            exit(EXIT_FAILURE);
        }
        
        if (strstr(buffer, MSG_SERVER_MENU)) {
            funzione_menu();
        }

    }

    close(sd);
    return 0;
}

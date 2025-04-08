#include "funzioni.h"
#include "strutture.h"

sd = 0;

int main() {
    inizializza_socket();
    char buffer[1024];
    while (1) {
        
        if (strstr(buffer, MSG_SERVER_MENU)) {
            funzione_menu();
        }

    }

    close(sd);
    return 0;
}

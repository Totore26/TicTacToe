#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    char buffer[1024];
    while (1) {
        recv(sock, buffer, sizeof(buffer), 0);
        printf("%s", buffer);

        if (strstr(buffer, "vinto") || strstr(buffer, "perso") || strstr(buffer, "Pareggio")) {
            break;
        }

        int row, col;
        scanf("%d %d", &row, &col);
        sprintf(buffer, "%d%d", row, col);
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}

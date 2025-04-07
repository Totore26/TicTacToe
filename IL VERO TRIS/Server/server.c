#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define SIZE 3  // Dimensione della griglia
#define MAX_CLIENTS 10

typedef struct {
    int player1;
    int player2;
} Game;

// Griglia di gioco
char board[SIZE][SIZE];

// Inizializza la griglia
void init_board() {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            board[i][j] = ' ';
}

// Controlla se un giocatore ha vinto
int check_winner(char mark) {
    for (int i = 0; i < SIZE; i++) {
        if (board[i][0] == mark && board[i][1] == mark && board[i][2] == mark) return 1;
        if (board[0][i] == mark && board[1][i] == mark && board[2][i] == mark) return 1;
    }
    if (board[0][0] == mark && board[1][1] == mark && board[2][2] == mark) return 1;
    if (board[0][2] == mark && board[1][1] == mark && board[2][0] == mark) return 1;
    return 0;
}

// Controlla se è pareggio
int is_draw() {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == ' ') return 0;
    return 1;
}

// Thread per gestire una partita
void *game_thread(void *arg) {
    Game *game = (Game *)arg;
    int players[2] = {game->player1, game->player2};
    char symbols[2] = {'X', 'O'};
    char buffer[1024];
    int current_player = 0;

    init_board();

    while (1) {
        sprintf(buffer, "Tocca a te, inserisci riga e colonna (0-2): ");
        send(players[current_player], buffer, strlen(buffer), 0);

        recv(players[current_player], buffer, sizeof(buffer), 0);
        int row = buffer[0] - '0', col = buffer[1] - '0';

        if (board[row][col] == ' ') {
            board[row][col] = symbols[current_player];
        } else {
            sprintf(buffer, "Mossa non valida, riprova.\n");
            send(players[current_player], buffer, strlen(buffer), 0);
            continue;
        }

        if (check_winner(symbols[current_player])) {
            sprintf(buffer, "Hai vinto!\n");
            send(players[current_player], buffer, strlen(buffer), 0);
            send(players[1 - current_player], "Hai perso!\n", 11, 0);
            break;
        }

        if (is_draw()) {
            sprintf(buffer, "Pareggio!\n");
            send(players[0], buffer, strlen(buffer), 0);
            send(players[1], buffer, strlen(buffer), 0);
            break;
        }

        current_player = 1 - current_player;
    }

    close(players[0]);
    close(players[1]);
    free(game);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);

    printf("Server in attesa di giocatori...\n");

    while (1) {
        int player1 = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        printf("Giocatore 1 connesso.\n");

        int player2 = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        printf("Giocatore 2 connesso. Avvio partita...\n");

        Game *game = malloc(sizeof(Game));
        game->player1 = player1;
        game->player2 = player2;

        pthread_t thread;
        pthread_create(&thread, NULL, game_thread, (void *)game);
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}

#include "headers/connectionFunctions.h"
#include "headers/gameFunctions.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define BUFSZ 1024

struct clientStatus {
    int run;
    int gamestatus; // salva o status atual do jogo, 1 c pode apostar 2 c pode
                    // dar cashout
    int hasBet;
    pthread_mutex_t mutex;
};
struct clientStatus session;

void *ReceiveTread(void *data) {
    int socket = *(int *)data;
    struct aviator_msg msg;
    while (session.run) {
        recv(socket, &msg, sizeof(msg), 0);
        if (strcmp(msg.type, "start") == 0) {
            session.gamestatus = 1;
            session.hasBet=0;
            printf("Rodada aberta! Digite o valor da aposta ou digite [Q] para "
                   "sair (%.2f segundos restantes): \n",
                   msg.value);
        } else if (strcmp(msg.type, "closed") == 0) {
            session.gamestatus = 0;
            printf("Apostas encerradas! Não é mais possível apostar nesta "
                   "rodada.\nDigite [C] para sacar.\n");
        } else if (strcmp(msg.type, "multiplier") == 0) {
            session.gamestatus = 2;
            printf("Multiplicador atual: %.2fx\n", msg.value);
        } else if (strcmp(msg.type, "explode") == 0) {
            session.gamestatus = 0;
            printf("Aviãozinho explodiu em: %.2fx\n", msg.value);
        } else if (strcmp(msg.type, "payout") == 0) {
            session.gamestatus = 0;
            printf("Você sacou e ganhou R$ %.2f!\n", msg.value);
        } else if (strcmp(msg.type, "profit") == 0) {
            session.gamestatus = 0;
            printf("Profit atual: R$ %.2f\nProfit da casa: R$ %.2f\n",
                   msg.player_profit, msg.house_profit);
        } else if (strcmp(msg.type, "bye") == 0) {
            session.gamestatus = 0;
            printf("O servidor caiu, mas sua esperança pode continuar de pé. "
                   "Até breve!\n");
            session.run = 0;
        }

        pthread_mutex_unlock(&session.mutex);
    }
}
void CheckInitialization(int argc, char **argv) {
    if (argc != 5) {
        LogExit("Error: Invalid number of arguments");
    }

    if (strcmp(argv[3], "-nick") != 0) {
        LogExit("Error: Expected '-nick' argument");
    }
    if (strlen(argv[4]) > 13) {
        LogExit("Error: Nickname toolong (max 13)");
    }
}

int main(int argc, char **argv) {
    CheckInitialization(argc, argv);
    session.run = 1;
    char *nick = argv[4];
    pthread_mutex_init(&session.mutex, NULL);

    struct sockaddr_storage storage;

    if (AddrParser(argv[1], argv[2], &storage) != 0) {
        LogExit("addrparser");
    }
    int s;
    // passa o storage.ss_family para inicializar com o IPV4, ou IPV6
    // corretamente
    s = socket(storage.ss_family, SOCK_STREAM, 0);

    if (s == -1) {
        LogExit("socket");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);

    if (0 != connect(s, addr, sizeof(storage))) {
        LogExit("connect");
    }

    struct aviator_msg mainMessage;
    pthread_t receiveTread;
    pthread_create(&receiveTread, NULL, ReceiveTread, &s);
    while (session.run) {
        char input[100];
        fgets(input, 100, stdin);
        pthread_mutex_lock(&session.mutex);
        int status = session.gamestatus;

        pthread_mutex_unlock(&session.mutex);

        if (input[0] == 'Q' || input[0] == 'q') {
            strcpy(mainMessage.type, "bye");
            send(s, &mainMessage, sizeof(mainMessage), 0);
            printf("Aposte com responsabilidade. A plataforma é nova e tá com "
                   "horário bugado. Volte logo, %s.\n",
                   nick);
            session.run = 0;
            break;
        } else if (input[0] == 'C' && status == 2) {
            strcpy(mainMessage.type, "cashout");
            printf("%s\n", mainMessage.type);
            send(s, &mainMessage, sizeof(mainMessage), 0);
        } else {
            if (status == 1 &&  !session.hasBet) {
                char *endptr;
                float bet = strtof(input, &endptr);
                bet = atof(input);
                 session.hasBet=1;
                if (endptr == input) {
                    printf("Error: Invalid command\n");
                    continue;
                }
                if (bet <= 0) {
                    printf("Error: Invalid bet value\n");
                    continue;
                }
                strcpy(mainMessage.type, "bet");
                mainMessage.value = bet;
                send(s, &mainMessage, sizeof(mainMessage), 0);
                printf("Aposta recebida: R$ %.2f\n", bet);
            }
        }
    }
    close(s);

    exit(EXIT_SUCCESS);
}
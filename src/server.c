#include "headers/connectionFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSZ 1024
#include <stdio.h>
/**
 * @brief Imprime a linha de log padrao do servidor, baseado na ordem apresentada no docs, quando um campo recebe valor 
 * 
 * A função exibe apenas os campos que foram fornecidos, ignorando os que receberam o parametro de que nao iram aparecer, 
 * pra maioria valor negativo menos para os lucros
 * 
 * @param event string obrigatória que indica o tipo de evento.
 * @param id identificador do jogador, ou NULL se não houver.
 * @param m multiplicador, valor negativo indica que não será exibido.
 * @param me ponto de explosão, valor negativo indica que não será exibido.
 * @param N número de jogadores, valor negativo indica que não será exibido.
 * @param V valor total apostado, valor negativo indica que não será exibido.
 * @param bet valor apostado, valor negativo indica que não será exibido.
 * @param payout valor recebido pelo jogador, valor negativo indica que não será exibido.
 * @param player_profit lucro acumulado do jogador, valor NULL indica que não será exibido.
 * @param house_profit lucro acumulado do servidor, valor NULL indica que não será exibido.
 */
void print_log(const char *event, const char *id, double m, int me, int N,
               double V, int bet, int payout, float player_profit,
               float house_profit) {
    printf("event=%s", event);

    if (id != NULL) {
        printf(" | id=%s", id);
    }
    if (m >= 0) {
        printf(" | m=%.2f", m);
    }
    if (me >= 0) {
        printf(" | me=%d", me);
    }
    if (N >= 0) {
        printf(" | N=%d", N);
    }
    if (V >= 0) {
        printf(" | V=%.2f", V);
    }
    if (bet >= 0) {
        printf(" | bet=%.2f", bet);
    }
    if (payout >= 0) {
        printf(" | payout=%.2f", payout);
    }
    if (player_profit != NULL) {
        printf(" | player_profit=%.2f", player_profit);
    }
    if (house_profit >= 0) {
        printf(" | house_profit=%.2f", house_profit);
    }

    printf("\n");
}

int main(int argc, char **argv) {
    if (argc < 3) {
        LogExit("argumentos");
    }
    struct sockaddr_storage storage;

    ServerSockaddrInit(argv[1], argv[2], &storage) != 0;

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);

    if (s == -1) {
        LogExit("socket");
    }
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    // bind->listen->accept
    if (bind(s, addr, sizeof(storage)) != 0) {
        LogExit("bind");
    }
    // segundo argumento sao o numero de conexoes pendentes
    if (listen(s, 10) != 0) {
        LogExit("listen");
    }

    char addrstr[BUFSZ];
    AddrToString(addr, addrstr, BUFSZ);
    printf("bound, to %s, waiting connection\n", addrstr);
    while (1) {
        struct sockaddr_storage clientStorage;
        struct sockaddr *clientAddr = (struct sockaddr *)(&storage);
        socklen_t clientAddrLen = sizeof(clientStorage);
        // s = nosso socket
        // clientAddr = endereço do cliente que a funçao sabe de onde veio
        // retorna o socket do cliente
        int clientSocket = accept(s, clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            LogExit("accept");
        }
        char clientAddrstr[BUFSZ];
        AddrToString(clientAddr, clientAddrstr, BUFSZ);
        printf("connection, from %s, \n", clientAddrstr);

        char buf[BUFSIZ];
        memset(buf, 0, BUFSIZ);
        size_t count = recv(clientSocket, buf, BUFSIZ - 1, 0);

        printf("[msg] %s, %d bytes : %s\n", clientAddrstr, (int)count, buf);

        sprintf(buf, "remote endpoint: %.1000s\n", clientAddrstr);
        count = send(clientSocket, buf, (strlen(buf) + 1), 0);
        if (count != strlen(buf) + 1) {
            LogExit("send");
        }
        close(clientSocket);
    }

    exit(EXIT_SUCCESS);
}
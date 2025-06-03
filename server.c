#include "headers/connectionFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSZ 1024

void Usage(int argc, char **argv) {
    printf("usage: %s <sv4>|| <v6>\n", argv[0]);
    printf("exemple: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        Usage(argc, argv);
    }
    struct sockaddr_storage storage;

    if (ServerSockaddrInit(argv[1], argv[2], &storage) != 0) {
        Usage(argc, argv);
    }
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
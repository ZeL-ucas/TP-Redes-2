#include "headers/connectionFunctions.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define BUFSZ 1024

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

    char *nick = argv[4];

    printf("%s\n", nick);

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

    // char addrstr[BUFSZ];
    // AddrToString(addr, addrstr, BUFSZ);

    // printf("connected to %s\n", addrstr);

    unsigned total = 0;
    while (1) {
        recv(s, total, BUFSZ - total, 0);
    }
    close(s);

    printf("receive %d bytes \n", total);
    exit(EXIT_SUCCESS);
}
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void LogExit(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int AddrParser(const char *addrstring, const char *portstr,
               struct sockaddr_storage *storage) {

    if (addrstring == NULL || portstr == NULL) {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr);

    if (port == 0) {
        return -1;
    }
    port = htons(port);

    struct in_addr inaddr4; // 32 bit ip

    if (inet_pton(AF_INET, addrstring, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // 128 bit ip

    if (inet_pton(AF_INET6, addrstring, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        // addr6->sin6_addr = inaddr6;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void AddrToString(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;
    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            LogExit("ntop");
        }
        port = ntohs(addr4->sin_port);
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            LogExit("ntop");
        }
        port = ntohs(addr6->sin6_port);

    } else {
        LogExit("unknow protocol");
    }
    if (str) {
        snprintf(str, strsize, "IPV%d %s %hu", version, addrstr, port);
    }
}

int ServerSockaddrInit(const char *proto, const char *portstr,
                       struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t)atoi(portstr);
    if (port == 0) {
        return -1;
    }
    port = htons(port);
    memset(storage, 0, sizeof(*storage));
    if (strcmp(proto, "v4") == 0) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;

        return 0;

    } else if (strcmp(proto, "v6") == 0) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;

        return 0;
    } else {
        return -1;
    }
}
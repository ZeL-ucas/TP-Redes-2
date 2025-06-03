#pragma once 

#include <inttypes.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

void LogExit(char *msg);

int AddrParser(const char *addrstring, const char *portstr,
               struct sockaddr_storage *storage);

void AddrToString(const struct sockaddr *addr, char *str, size_t strsize);

int ServerSockaddrInit(const char *proto, const char *portstr,
                       struct sockaddr_storage *storage);

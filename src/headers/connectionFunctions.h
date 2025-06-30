#pragma once 

#include <inttypes.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#define STR_LEN 11
struct aviator_msg {
    int32_t player_id;
    float value;
    char type[STR_LEN];
    float player_profit;
    float house_profit;
};
void LogExit(char *msg);
/**
 * @brief converte uma string de endereço IP e porta para sockaddr_storage, pega das videoaulas disponibilizadas.
 *
 * @param addrstring string contendo o endereço IP (IPv4 ou IPv6).
 * @param portstr string contendo a porta.
 * @param storage ponteiro para a struct sockaddr_storage a ser preenchida.
 * @return int 0 em sucesso, diferente de 0 em erro.
 */
int AddrParser(const char *addrstring, const char *portstr,
               struct sockaddr_storage *storage);
/**
 * @brief converte um endereço sockaddr em string , pega das videoaulas disponibilizadas.
 *
 * @param addr ponteiro para a struct sockaddr.
 * @param str buffer onde será armazenada a string formatada.
 * @param strsize tamanho do buffer str.
 * @return void
 */
void AddrToString(const struct sockaddr *addr, char *str, size_t strsize);
/**
 * @brief inicializa uma struct sockaddr_storage para servidor, pega das videoaulas disponibilizadas
 *
 * tambem configura porta e IP
 *
 * @param proto string que especifica o protocolo ("v4" ou "v6").
 * @param portstr porta do servidor como string.
 * @param storage ponteiro para sockaddr_storage que será inicializado.
 * @return int 0 em sucesso, diferente de 0 em erro.
 */
int ServerSockaddrInit(const char *proto, const char *portstr,
                       struct sockaddr_storage *storage);
#include "headers/connectionFunctions.h"
#include "headers/gameFunctions.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>

struct GameState currentGame;

void *ShutdownServer(void *data) {
    int socket = *(int *)data;
    while (1) {
        char input = '0';
        scanf(" %c", &input);
        if (input == 'Q' || input == 'q') {
            pthread_mutex_lock(&currentGame.mutex);
            for (int i = 0; i < currentGame.NumberOfClients; i++) {
                logoutCliente(&currentGame, i);
            }
            close(socket);
            pthread_mutex_unlock(&currentGame.mutex);
            exit(EXIT_SUCCESS);
        }
    }
}
/**
 @brief tread principal para gerenciar o jogo/round, apesar de ter um parametro
 ele nao é utilizado (só esta pelas regras de tread)
 **/
void *MainTread(void *data) {
    int hasClients = 0;
    while (1) {
        while (1) {
            pthread_mutex_lock(&currentGame.mutex);
            printf("%d\n", currentGame.NumberOfClients);
            hasClients = currentGame.NumberOfClients;
            pthread_mutex_unlock(&currentGame.mutex);
            sleep(1);
            if (hasClients) {
                break;
            }
        }
        pthread_mutex_lock(&currentGame.mutex);
        PrintLog("start", "*", -1, -1, currentGame.NumberOfClients, -1, -1, -1,
                 NULL, NULL);
        struct aviator_msg startMsg;
        strcpy(startMsg.type,"start");
        startMsg.value = 10;
        SendToAll(&startMsg,&currentGame);
        currentGame.gameStarted = 1;
        currentGame.bettingOpen = 1;
        currentGame.GameRunning = 0;
        pthread_mutex_unlock(&currentGame.mutex);

        for (int i = 10; i > 0; i--) {
            pthread_mutex_lock(&currentGame.mutex);
            currentGame.bettingWindow = (float)i;
            pthread_mutex_unlock(&currentGame.mutex);
            printf("%d\n", i);
            usleep(1000000);
        }

        pthread_mutex_lock(&currentGame.mutex);

        float maxMultiplier = GenerateMultiplier(&currentGame);
        PrintLog("closed", "*", -1, -1, currentGame.NumberOfClients,
                 currentGame.totalBetValue, -1, -1, NULL, NULL);
        struct aviator_msg closeMsg;
        closeMsg.value = 0;
        strcpy(closeMsg.type, "closed");
        SendToAll(&closeMsg, &currentGame);
        printf("%.2f", maxMultiplier);
        currentGame.GameRunning = 1;
        pthread_mutex_unlock(&currentGame.mutex);

        float multiplier = 1.00;
        while (multiplier < maxMultiplier) {
            struct aviator_msg multiplierMsg;
            strcpy(multiplierMsg.type, "multiplier");
            multiplierMsg.value = multiplier;
            pthread_mutex_lock(&currentGame.mutex);
            SendToAllBetters(&multiplierMsg, &currentGame);
            currentGame.currentMultiplier = multiplier;
            pthread_mutex_unlock(&currentGame.mutex);
            PrintLog("multiplier", "*", multiplier, -1, -1, -1, -1, -1, NULL,
                     NULL);
            usleep(100000);
            multiplier += 0.01;
        }
        pthread_mutex_lock(&currentGame.mutex);
        currentGame.GameRunning = 0;
        PrintLog("explode", "*", multiplier, -1, -1, -1, -1, -1, NULL, NULL);
        pthread_mutex_unlock(&currentGame.mutex);
        for (int i = 0; i < currentGame.NumberOfClients; i++) {
            pthread_mutex_lock(&currentGame.mutex);
            struct ClientData *client = currentGame.clients[i];
            if (client->alreadyBet && !client->alreadyCashedOut) {
                printf("estou aqui");
                client->playerProfit -= client->bet;
                currentGame.houseProfit += client->bet;

                struct aviator_msg explodeMsg;
                explodeMsg.player_id = client->Id;
                explodeMsg.value = currentGame.currentMultiplier;
                strcpy(explodeMsg.type, "explode");
                explodeMsg.player_profit = client->playerProfit;
                explodeMsg.house_profit = currentGame.houseProfit;
                send(client->clientSocket, &explodeMsg, sizeof(explodeMsg), 0);
                struct aviator_msg profitMsg;
                profitMsg.player_id = client->Id;
                strcpy(profitMsg.type, "profit");
                profitMsg.value = 0;
                profitMsg.player_profit = client->playerProfit;
                profitMsg.house_profit = currentGame.houseProfit;
                send(client->clientSocket, &profitMsg, sizeof(profitMsg), 0);
                PrintLog("explode", &client->Id, currentGame.currentMultiplier,
                         -1, -1, -1, -1, -1, NULL, NULL);
                PrintLog("profit", &client->Id, -1, -1, -1, -1, -1,
                         client->playerProfit, NULL, NULL);
            }
            pthread_mutex_unlock(&currentGame.mutex);
        }
        PrintLog("profit", "*", -1, -1, -1, -1, -1, -1, NULL,
                 &currentGame.houseProfit);
        pthread_mutex_lock(&currentGame.mutex);
        for (int i = 0; i < currentGame.NumberOfClients; i++) {
            currentGame.clients[i]->alreadyBet = 0;
            currentGame.clients[i]->alreadyCashedOut = 0;
            currentGame.clients[i]->bet = 0;
        }
        currentGame.totalBetValue = 0;

        pthread_mutex_unlock(&currentGame.mutex);
    }
}
/**
 @brief tread individual de cada client, recebe como parametro a struct de dados
 daquele client
 **/
void *ClientTread(void *data) {
    struct ClientData *clientData = (struct ClientData *)data;
    currentGame.clients[clientData->numberOnArray]->bet = 0;
    while (1) {
        pthread_mutex_lock(&currentGame.mutex);
        if (!currentGame.gameStarted) {
            pthread_mutex_unlock(&currentGame.mutex);
            sleep(1);
            continue;
        }
        if (!clientData->alreadyBet) {
            if (currentGame.bettingOpen) {
                strcpy(clientData->mainMessage.type, "start");
                clientData->mainMessage.value = currentGame.bettingWindow;
                send(clientData->clientSocket, &clientData->mainMessage,
                     sizeof(clientData->mainMessage), 0);
            } else {
                strcpy(clientData->mainMessage.type, "closed");
                send(clientData->clientSocket, &clientData->mainMessage,
                     sizeof(clientData->mainMessage), 0);
            }
        }
        pthread_mutex_unlock(&currentGame.mutex);
        recv(clientData->clientSocket, &clientData->mainMessage,
             sizeof(clientData->mainMessage), 0);
        printf("%s\n", clientData->mainMessage.type);
        pthread_mutex_lock(&currentGame.mutex);
        if (strcmp(clientData->mainMessage.type, "bet") == 0) {
            if (!clientData->alreadyBet && currentGame.bettingOpen) {
                clientData->bet = clientData->mainMessage.value;

                clientData->alreadyBet = 1;
                clientData->alreadyCashedOut = 0;

                currentGame.totalBetValue += clientData->bet;
                currentGame.clients[clientData->numberOnArray]->bet =
                    clientData->bet;
                PrintLog("bet", &clientData->Id, -1, -1,
                         currentGame.NumberOfClients, currentGame.totalBetValue,
                         clientData->bet, -1, NULL, NULL);
            }
        } else if (strcmp(clientData->mainMessage.type, "cashout") == 0) {
            printf("ola %d %d\n",clientData->alreadyBet,clientData->alreadyCashedOut);
            if (clientData->alreadyBet && !clientData->alreadyCashedOut &&
                currentGame.GameRunning) {
                clientData->alreadyCashedOut = 1;
                float payout = clientData->bet * currentGame.currentMultiplier;
                clientData->playerProfit += (payout - clientData->bet);
                currentGame.houseProfit -= (payout - clientData->bet);
                struct aviator_msg payoutMsg;
                payoutMsg.player_id = clientData->Id;
                payoutMsg.value = payout;
                strcpy(payoutMsg.type, "payout");
                payoutMsg.player_profit = clientData->playerProfit;
                payoutMsg.house_profit = currentGame.houseProfit;
                send(clientData->clientSocket, &payoutMsg, sizeof(payoutMsg),
                     0);
                struct aviator_msg profitMsg;
                profitMsg.player_id = clientData->Id;
                strcpy(profitMsg.type, "profit");
                profitMsg.value = 0;
                profitMsg.player_profit = clientData->playerProfit;
                profitMsg.house_profit = currentGame.houseProfit;
                send(clientData->clientSocket, &profitMsg, sizeof(profitMsg),
                     0);
                PrintLog("cashout", &clientData->Id,
                         currentGame.currentMultiplier, -1, -1, -1, -1, -1,
                         NULL, NULL);
                PrintLog("payout", &clientData->Id, -1, -1, -1, -1, payout, -1,
                         NULL, NULL);
                PrintLog("profit", &clientData->Id, -1, -1, -1, -1, -1, -1,
                         &clientData->playerProfit, NULL);
            }
        } else if (strcmp(clientData->mainMessage.type, "bye") == 0) {
            logoutCliente(&currentGame, clientData->numberOnArray);
            
            pthread_mutex_unlock(&currentGame.mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&currentGame.mutex);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        LogExit("argumentos");
    }
    struct sockaddr_storage storage;
    pthread_mutex_init(&currentGame.mutex, NULL);
    currentGame.NumberOfClients = 0;
    if (ServerSockaddrInit(argv[1], argv[2], &storage) != 0) {
        LogExit("ServerSockaddrInit failed");
    }
    pthread_t gameTread;
    pthread_t shutdownTread;

    pthread_create(&gameTread, NULL, MainTread, NULL);
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    pthread_create(&shutdownTread, NULL, ShutdownServer, &s);
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
    currentGame.gameStarted = 0;
    int nextId = 1;
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
        pthread_mutex_lock(&currentGame.mutex);
        int currentClients = currentGame.NumberOfClients;
        pthread_mutex_unlock(&currentGame.mutex);
        if (currentClients < 10) {
            pthread_mutex_lock(&currentGame.mutex);
            struct ClientData *clientData = malloc(sizeof(*clientData));
            clientData->clientSocket = clientSocket;

            memcpy(&(clientData->storage), &storage, sizeof(storage));
            clientData->Id = nextId++;
            clientData->numberOnArray = currentGame.NumberOfClients;
            clientData->bet = 0;
            clientData->alreadyBet = 0;
            clientData->alreadyCashedOut = 0;
            clientData->playerProfit = 0;
            currentGame.clients[currentGame.NumberOfClients] = clientData;
            currentGame.NumberOfClients++;

            pthread_t clientTread;
            pthread_create(&clientTread, NULL, ClientTread, clientData);
            pthread_mutex_unlock(&currentGame.mutex);
        } else {
            printf("numero maximo de clientes");
        }
    }
    close(s);
    exit(EXIT_SUCCESS);
}

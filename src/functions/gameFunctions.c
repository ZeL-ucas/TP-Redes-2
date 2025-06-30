#include "../headers/gameFunctions.h"
#include <math.h>
#include <sys/socket.h>

void PrintLog(const char *event, const char *id, double multiplier,
              int explosion, int N, double V, double bet, double payout,
              float *playerProfit, float *houseProfit) {
    printf("event=%s", event);

    if (id != NULL) {
        printf(" | id=%s", id);
    }
    if (multiplier >= 0) {
        printf(" | m=%.2f", multiplier);
    }
    if (explosion >= 0) {
        printf(" | me=%d", explosion);
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
    if (playerProfit != NULL) {
        printf(" | player_profit=%.2f", *playerProfit);
    }
    if (houseProfit != NULL) {
        printf(" | house_profit=%.2f", *houseProfit);
    }

    printf("\n");
    fflush(stdout);
}

void SendToAll(struct aviator_msg *msg, struct GameState *gameState) {
    for (int i = 0; i < gameState->NumberOfClients; i++) {
        send(gameState->clients[i]->clientSocket, msg,
             sizeof(struct aviator_msg), 0);
    }
}

float GenerateMultiplier(struct GameState *gameState) {
    float totalBetValue = 0;
    int totalBetters = 0;
    printf("%d %f \n", gameState->clients[0]->Id, gameState->clients[0]->bet);
    float multiplier;
    printf("%f %d\n", totalBetValue, totalBetters);
    for (int i = 0; i < gameState->NumberOfClients; i++) {
        printf(" aposta %f\n ", gameState->clients[i]->bet);
        if (gameState->clients[i]->bet > 0) {
            totalBetters++;
            totalBetValue += gameState->clients[i]->bet;
        }
    }
    // formula do enunciado
    gameState->totalBetValue = totalBetValue;
    totalBetValue *= 0.01;
    multiplier = 1 + totalBetters + totalBetValue;
    multiplier = sqrt(multiplier);
    printf("%f", multiplier);
    return multiplier;
}

void SendToAllBetters(struct aviator_msg *msg, struct GameState *gameState) {
    for (int i = 0; i < gameState->NumberOfClients; i++) {
        if (gameState->clients[i]->bet > 0 &&
            !gameState->clients[i]->alreadyCashedOut) {
            send(gameState->clients[i]->clientSocket, msg,
                 sizeof(struct aviator_msg), 0);
        }
    }
}

void logoutCliente(struct GameState *gameState, int index) {
    struct ClientData *client = gameState->clients[index];
    client->mainMessage.player_id = client->Id;
    char id = client->Id;
    strcpy(client->mainMessage.type, "bye");
    client->mainMessage.value = 0;
    client->mainMessage.player_profit = client->playerProfit;
    client->mainMessage.house_profit = gameState->houseProfit;
    send(client->clientSocket, &client->mainMessage,
         sizeof(client->mainMessage), 0);
    close(client->clientSocket);
    free(client);

    for (int i = index; i < gameState->NumberOfClients - 1; i++) {
        gameState->clients[i] = gameState->clients[i + 1];
        gameState->clients[i]->numberOnArray = i;
    }

    gameState->NumberOfClients--;
    PrintLog("bye", &id, -1, -1, -1, -1, -1, -1, NULL, NULL);
}
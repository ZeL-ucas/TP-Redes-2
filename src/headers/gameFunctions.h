#include "connectionFunctions.h"
#include <pthread.h>
#include <sys/socket.h>
#include "connectionFunctions.h"
/**
 * @brief estrutura para armazenar os dados de um client especifico

 */
struct ClientData {
    int clientSocket; //socket do client
    struct sockaddr_storage storage; // storage
    char Id; //id desse client 
    struct aviator_msg mainMessage; //menssagem que ele esta trocando com o servidor
    float bet; //quantidade que ele apostou na rodada atual

    int numberOnArray; //guarda a posiçao no array global do game
    int alreadyBet;
    int alreadyCashedOut;
    float playerProfit;

    };
/**
 * @brief struct para armazenar o estado do jogo, será uzada como variavel global para todas as treads terem acesso

 */
struct GameState {
    int NumberOfClients; //numero de clientes atuais, maximo 10
    struct ClientData* clients[10]; //dados de cada client(expressos na struct CLientData), limitado a 10
    int bettingOpen; //mostra se as apostas estao abertas, 1 caso sim, 0 caso nao
    int GameRunning; //mostra se a rodada esta rodando, 1 caso sim, 0 caso nao
    float bettingWindow; //guarda o momento que a rodada começou para poder calcular o multiplicador 
    pthread_mutex_t mutex; //mutex utilizado para travar execuçoes quando necessario 
    float totalBetValue;
    float currentMultiplier;
    float houseProfit;
        int gameStarted;
    };
/**
 * @brief Imprime a linha de log padrao do servidor, baseado na ordem apresentada no docs, quando um campo recebe valor 
 * 
 * A função exibe apenas os campos que foram fornecidos, ignorando os que receberam o parametro de que nao iram aparecer, 
 * pra maioria valor negativo menos para os lucros
 * 
 * @param event string obrigatória que indica o tipo de evento.
 * @param id identificador do jogador, ou NULL se não houver.
 * @param multiplier multiplicador, valor negativo indica que não será exibido.
 * @param explosion ponto de explosão, valor negativo indica que não será exibido.
 * @param N número de jogadores, valor negativo indica que não será exibido.
 * @param V valor total apostado, valor negativo indica que não será exibido.
 * @param bet valor apostado, valor negativo indica que não será exibido.
 * @param payout valor recebido pelo jogador, valor negativo indica que não será exibido.
 * @param playerProfit lucro acumulado do jogador, valor NULL indica que não será exibido.
 * @param houseProfit lucro acumulado do servidor, valor NULL indica que não será exibido.
 */
void PrintLog(const char *event, const char *id, double multiplier, int explosion, int N,
               double V, double bet, double payout, float *playerProfit,
               float *houseProfit);
/**
 * @brief manda mensagem para todos os clientes, basicamente o broadcast
 * 
 * @param msg mensagem a ser enviada para todos os jogadores.
 * @param gameState struct que guarda o vetor de usuarios atuais.

 */
void SendToAll(struct aviator_msg *msg, struct GameState *gameState);
/**
 * @brief Gera o multiplicador e o retorna
 * 
 * @param gameState struct que guarda o vetor de usuarios atuais.

 */
float GenerateMultiplier(struct GameState *gameState);

/**
 * @brief manda mensagem para todos os apostadores da rodada
 * 
 * @param msg mensagem a ser enviada para todos os jogadores.
 * @param gameState struct que guarda o vetor de usuarios atuais.

 */
void SendToAllBetters(struct aviator_msg *msg, struct GameState *gameState);
/**
 * @brief desconecta um cliente, da free na memoria, manda mensagem de bye e desloca o vetor 
 * 
 * @param gameState struct que guarda o vetor de usuarios atuais.
 * @param index index do cliente que vai ser desconectado
 */
void logoutCliente(struct GameState *game, int index);
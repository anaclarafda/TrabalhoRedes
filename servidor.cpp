#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <map>   // Para armazenar pacotes ordenadamente
#include <ctime> // Para gerar perdas aleatórias

#pragma comment(lib, "ws2_32.lib")
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define JANELA_RECEBIMENTO 32 // Tamanho da janela de recepção
#define TAXA_PERDA 10         // Percentual de pacotes descartados (ex: 10%)

int main()
{
    WSADATA wsaData;
    SOCKET serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Erro ao inicializar o Winsock." << std::endl;
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Erro ao criar o socket." << std::endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Erro ao associar o socket ao endereco." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor aguardando pacotes..." << std::endl;

    std::map<int, std::string> pacotesRecebidos;
    int ultimoPacoteRecebido = 0;
    srand(time(0));

    while (true)
    {
        int bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0,
                                     (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR)
        {
            std::cerr << "Erro ao receber dados." << std::endl;
            continue;
        }

        // Extrai número do pacote e dados
        int numeroPacote;
        char dados[BUFFER_SIZE];
        sscanf(buffer, "%d|%s", &numeroPacote, dados);

        // Simula perda de pacotes com uma chance de TAXA_PERDA%
        if (rand() % 100 < TAXA_PERDA)
        {
            std::cerr << "Pacote " << numeroPacote << " PERDIDO!" << std::endl;
            continue; // Ignora o processamento do pacote
        }

        // Armazena pacotes na ordem correta
        pacotesRecebidos[numeroPacote] = dados;

        // Confirmação acumulativa (ACK acumulativo)
        ultimoPacoteRecebido = numeroPacote;
        char ackMessage[50];
        sprintf(ackMessage, "ACK %d", ultimoPacoteRecebido);
        sendto(serverSocket, ackMessage, sizeof(ackMessage), 0,
               (struct sockaddr *)&clientAddr, clientAddrLen);

        // Informa a janela de recepção ao cliente
        char janelaMessage[50];
        sprintf(janelaMessage, "JANELA %d", JANELA_RECEBIMENTO);
        sendto(serverSocket, janelaMessage, sizeof(janelaMessage), 0,
               (struct sockaddr *)&clientAddr, clientAddrLen);

        std::cout << "Pacote " << numeroPacote << " recebido e confirmado com ACK." << std::endl;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

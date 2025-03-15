#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <map>
#include <ctime>
#include <cstdlib> // Para gerar perdas aleatórias

#pragma comment(lib, "ws2_32.lib")
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define PERDA_PERCENTUAL 10 // % de pacotes que serão descartados aleatoriamente

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
        std::cerr << "Erro ao associar o socket ao endereço." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor aguardando pacotes..." << std::endl;

    std::map<int, std::string> pacotesRecebidos;
    int ultimoPacoteContiguo = 0; // Último pacote recebido em ordem

    srand(time(0)); // Inicializa a semente do gerador de números aleatórios

    while (true)
    {
        int bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0,
                                     (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR)
        {
            std::cerr << "Erro ao receber dados." << std::endl;
            continue;
        }

        int numeroPacote;
        char dados[BUFFER_SIZE];
        sscanf(buffer, "%d|%s", &numeroPacote, dados);

        // Simulação de perda de pacotes
        if ((rand() % 100) < PERDA_PERCENTUAL)
        {
            std::cerr << "Pacote " << numeroPacote << " perdido (simulacao)!" << std::endl;
            continue;
        }

        pacotesRecebidos[numeroPacote] = dados;

        // Verifica se recebemos o próximo pacote esperado
        if (numeroPacote == ultimoPacoteContiguo + 1)
        {
            ultimoPacoteContiguo = numeroPacote;

            // Atualiza o último pacote contíguo recebido corretamente
            while (pacotesRecebidos.count(ultimoPacoteContiguo + 1))
            {
                ultimoPacoteContiguo++;
            }
        }

        // Enviar ACK acumulativo para o cliente
        char ackMsg[50];
        sprintf(ackMsg, "ACK %d", ultimoPacoteContiguo);
        sendto(serverSocket, ackMsg, sizeof(ackMsg), 0,
               (struct sockaddr *)&clientAddr, clientAddrLen);

        std::cout << "Pacote " << numeroPacote << " recebido. Enviando ACK " << ultimoPacoteContiguo << "." << std::endl;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

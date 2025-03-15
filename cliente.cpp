#include <iostream>
#include <winsock2.h>
#include <map>
#include <ctime>

#define SERVER "127.0.0.1"
#define PORT 8080
#define BUF_SIZE 1024
#define TOTAL_PACOTES 10000

int main()
{
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char pacote[BUF_SIZE], ackBuffer[BUF_SIZE];
    int serverAddrLen = sizeof(serverAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Erro ao inicializar o Winsock." << std::endl;
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Erro ao criar o socket." << std::endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER);

    int ultimoACKRecebido = 0;
    std::map<int, bool> pacotesEnviados;
    int janelaEnvio = 5;

    for (int i = 1; i <= TOTAL_PACOTES; i++)
    {
        pacotesEnviados[i] = false;
    }

    while (ultimoACKRecebido < TOTAL_PACOTES)
    {
        int enviados = 0;
        for (int i = ultimoACKRecebido + 1; i <= TOTAL_PACOTES && enviados < janelaEnvio; i++)
        {
            if (!pacotesEnviados[i])
            {
                std::sprintf(pacote, "%d|Pacote %d", i, i);
                sendto(clientSocket, pacote, sizeof(pacote), 0,
                       (struct sockaddr *)&serverAddr, sizeof(serverAddr));
                std::cout << "Pacote " << i << " enviado." << std::endl;
                pacotesEnviados[i] = true;
                enviados++;
            }
        }

        int bytesReceived = recvfrom(clientSocket, ackBuffer, BUF_SIZE, 0,
                                     (struct sockaddr *)&serverAddr, &serverAddrLen);
        if (bytesReceived > 0)
        {
            int novoACK, novaJanela;
            sscanf(ackBuffer, "ACK %d JANELA %d", &novoACK, &novaJanela);
            if (novoACK > ultimoACKRecebido)
            {
                ultimoACKRecebido = novoACK;
                janelaEnvio = novaJanela;
                std::cout << "ACK " << novoACK << " recebido. Nova janela: " << janelaEnvio << std::endl;
            }
        }
        else
        {
            std::cout << "Timeout! Reenviando pacotes..." << std::endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

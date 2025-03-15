#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <cstring>
#include <ctime>
#include <map>

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

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Erro ao inicializar o Winsock." << std::endl;
        return 1;
    }

    // Cria socket UDP
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Erro ao criar o socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Configura endereço do servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER);

    std::ofstream logFile("pacotes_enviados.txt");
    if (!logFile.is_open())
    {
        std::cerr << "Erro ao abrir o arquivo para gravar os pacotes enviados!" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    int ultimoACKRecebido = 0;
    std::map<int, bool> pacotesEnviados;

    for (int i = 1; i <= TOTAL_PACOTES; i++)
    {
        pacotesEnviados[i] = false; // Inicialmente nenhum pacote foi confirmado
    }

    int tentativa = 0;
    while (ultimoACKRecebido < TOTAL_PACOTES)
    {
        for (int i = ultimoACKRecebido + 1; i <= TOTAL_PACOTES; i++)
        {
            if (!pacotesEnviados[i])
            {
                std::sprintf(pacote, "%d|Pacote %d", i, i);
                sendto(clientSocket, pacote, sizeof(pacote), 0,
                       (struct sockaddr *)&serverAddr, sizeof(serverAddr));
                logFile << i << "," << std::time(nullptr) << std::endl;
                std::cout << "Pacote " << i << " enviado." << std::endl;
                pacotesEnviados[i] = true;
            }
        }

        // Aguarda ACK
        int bytesReceived = recvfrom(clientSocket, ackBuffer, BUF_SIZE, 0,
                                     (struct sockaddr *)&serverAddr, &serverAddrLen);
        if (bytesReceived > 0)
        {
            int novoACK;
            sscanf(ackBuffer, "ACK %d", &novoACK);
            if (novoACK > ultimoACKRecebido)
            {
                ultimoACKRecebido = novoACK;
                std::cout << "ACK " << novoACK << " recebido." << std::endl;
            }
        }
        else
        {
            tentativa++;
            std::cout << "Timeout! Reenviando pacotes..." << std::endl;
            if (tentativa >= 3)
            {
                std::cerr << "Erro: servidor não responde!" << std::endl;
                break;
            }
        }
    }

    logFile.close();
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

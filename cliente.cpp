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

    // Variáveis do Controle de Congestionamento
    int cwnd = 1;          // Janela de Congestionamento (inicialmente 1)
    int ssthresh = 16;     // Limite de Slow Start
    int ackDuplicados = 0; // Contador de ACKs duplicados
    int ultimoACK = -1;    // Último ACK recebido

    for (int i = 1; i <= TOTAL_PACOTES; i++)
    {
        pacotesEnviados[i] = false;
    }

    while (ultimoACKRecebido < TOTAL_PACOTES)
    {
        int enviados = 0;
        for (int i = ultimoACKRecebido + 1; i <= TOTAL_PACOTES && enviados < cwnd; i++)
        {
            if (!pacotesEnviados[i])
            {
                std::sprintf(pacote, "%d|Pacote %d", i, i);
                sendto(clientSocket, pacote, sizeof(pacote), 0,
                       (struct sockaddr *)&serverAddr, sizeof(serverAddr));
                std::cout << "Pacote " << i << " enviado (cwnd=" << cwnd << ")." << std::endl;
                pacotesEnviados[i] = true;
                enviados++;
            }
        }

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

                // Se estamos na fase de Slow Start
                if (cwnd < ssthresh)
                {
                    cwnd *= 2; // Crescimento exponencial
                }
                else
                {
                    cwnd += 1; // Crescimento linear (Congestion Avoidance)
                }

                ackDuplicados = 0; // Reseta o contador de ACKs duplicados
            }
            else if (novoACK == ultimoACK)
            {
                ackDuplicados++;
                if (ackDuplicados == 3)
                { // Se recebeu 3 ACKs duplicados, assume perda
                    std::cout << "Perda detectada! Reduzindo cwnd." << std::endl;
                    ssthresh = cwnd / 2;
                    cwnd = ssthresh;
                    ackDuplicados = 0;
                }
            }
            else
            {
                ackDuplicados = 0;
            }

            ultimoACK = novoACK;
        }
        else
        {
            std::cout << "Timeout! Reduzindo cwnd." << std::endl;
            ssthresh = cwnd / 2;
            cwnd = 1; // Volta para Slow Start
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

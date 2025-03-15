#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <cstring>
#include <ctime>

#define SERVER "127.0.0.1"
#define PORT 8080
#define BUF_SIZE 1024
#define TIMEOUT 100 // Tempo de espera por ACK em milissegundos

int main()
{
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char pacote[BUF_SIZE];
    int totalPacotes = 10000;    // Total de pacotes a enviar
    int cwnd = 1, ssthresh = 64; // Controle de congestionamento

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

    std::ofstream logFile("pacotes_enviados.txt");
    if (!logFile.is_open())
    {
        std::cerr << "Erro ao abrir o arquivo de log!" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::ifstream file("teste.bin", std::ios::binary);
    if (!file)
    {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return 1;
    }

    int numPacote = 1, ackEsperado = 0;
    char ackBuffer[50];
    struct timeval timeout = {0, TIMEOUT * 1000};
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    clock_t startTime = clock();
    int retransmissoes = 0;

    while (file.read(pacote, sizeof(pacote)) || numPacote <= totalPacotes)
    {
        std::sprintf(pacote, "%d|%s", numPacote, pacote);
        sendto(clientSocket, pacote, sizeof(pacote), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        logFile << numPacote << "," << std::time(nullptr) << std::endl;
        std::cout << "Pacote " << numPacote << " enviado" << std::endl;

        // Aguarda ACK do servidor
        int serverLen = sizeof(serverAddr);
        int bytesReceived = recvfrom(clientSocket, ackBuffer, sizeof(ackBuffer), 0, (struct sockaddr *)&serverAddr, &serverLen);

        if (bytesReceived > 0)
        {
            int ackRecebido;
            sscanf(ackBuffer, "ACK %d", &ackRecebido);
            ackEsperado = ackRecebido + 1;

            if (cwnd < ssthresh)
                cwnd *= 2; // Slow Start
            else
                cwnd++; // Congestion Avoidance
        }
        else
        {
            std::cerr << "Timeout! Retransmitindo pacote " << numPacote << std::endl;
            ssthresh = cwnd / 2;
            cwnd = 1;
            retransmissoes++;
        }

        numPacote++;
    }

    clock_t endTime = clock();
    double tempoTotal = double(endTime - startTime) / CLOCKS_PER_SEC;

    std::cout << "Transmissao concluida em " << tempoTotal << " segundos." << std::endl;
    std::cout << "Total de retransmissoes: " << retransmissoes << std::endl;

    logFile.close();
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

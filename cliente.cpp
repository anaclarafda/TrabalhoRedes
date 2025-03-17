#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <cstring>
#include <ctime>
#include <chrono>
#include <thread>

#define SERVER "127.0.0.1"
#define PORT 8080
#define BUF_SIZE 1024
#define TOTAL_PACOTES 10000  // Total de pacotes a enviar
#define LOSS_PROBABILITY 0.1 // Probabilidade de perda de pacotes

// Função para simular a perda de pacotes
bool simulate_packet_loss()
{
    return (rand() % 100) < (LOSS_PROBABILITY * 100);
}

int main()
{
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char pacote[BUF_SIZE];
    char resposta[BUF_SIZE];
    int pacotesEnviados = 0; // Contagem de pacotes enviados

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Erro ao inicializar o Winsock." << std::endl;
        return 1;
    }

    // Cria o socket do cliente
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Erro ao criar o socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Prepara o endereço do servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER);

    // Abrir o arquivo para registrar os pacotes enviados
    std::ofstream logFile("pacotes_enviados.txt");
    if (!logFile.is_open())
    {
        std::cerr << "Erro ao abrir o arquivo para gravar os pacotes enviados!" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Controle de congestionamento
    int cwnd = 1;            // Tamanho inicial da janela
    int ssthresh = 64;       // Limite do slow-start
    int pacotesPerdidos = 0; // Contador de pacotes perdidos

    // Loop para enviar os pacotes até atingir o total
    while (pacotesEnviados < TOTAL_PACOTES)
    {
        // Preenche o pacote com número de sequência e dados
        std::sprintf(pacote, "%d|Pacote %d", pacotesEnviados + 1, pacotesEnviados + 1);

        // Simula a perda de pacotes
        bool pacotePerdido = simulate_packet_loss();

        if (pacotePerdido)
        {
            pacotesPerdidos++;
            // Reduz o cwnd e ssthresh em caso de perda
            cwnd = cwnd / 2;
            ssthresh = cwnd;
            std::cout << "Perda de pacote! Reduzindo cwnd para " << cwnd << " e ssthresh para " << ssthresh << std::endl;
        }
        else
        {
            // Aumenta a janela se o pacote foi bem recebido
            if (cwnd < ssthresh)
            {
                cwnd++; // Fase de Slow Start
            }
            else
            {
                cwnd++; // Fase de Congestion Avoidance (aumenta linearmente)
            }

            std::cout << "Enviando pacote " << pacotesEnviados + 1 << " com cwnd = " << cwnd << std::endl;
        }

        // Envia o pacote
        sendto(clientSocket, pacote, sizeof(pacote), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

        // Registra o envio
        std::time_t currentTime = std::time(nullptr);
        logFile << pacotesEnviados + 1 << "," << currentTime << std::endl;
        std::cout << "Pacote " << pacotesEnviados + 1 << " enviado" << std::endl;

        // Aguarda a confirmação do servidor (Janela de recepção)
        int bytesReceived = recvfrom(clientSocket, resposta, BUF_SIZE, 0, NULL, NULL);
        if (bytesReceived != SOCKET_ERROR)
        {
            std::cout << "Recebido ACK: " << resposta << std::endl;
        }
        else
        {
            std::cerr << "Erro ao receber ACK." << std::endl;
        }

        pacotesEnviados++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10 milissegundos para simular latência
    }

    logFile.close();
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

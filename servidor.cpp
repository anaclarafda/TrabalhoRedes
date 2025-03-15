#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <map>

#pragma comment(lib, "ws2_32.lib")
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define JANELA_MAXIMA 5 // Tamanho da janela de recepção do servidor

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
    int ultimoACK = 0;
    int janelaAtual = JANELA_MAXIMA;

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

        pacotesRecebidos[numeroPacote] = dados;
        std::cout << "Pacote " << numeroPacote << " recebido." << std::endl;

        // Ajuste da janela: Se o servidor estiver sobrecarregado, reduz a janela
        if (pacotesRecebidos.size() > JANELA_MAXIMA)
        {
            janelaAtual = std::max(1, janelaAtual - 1);
        }
        else
        {
            janelaAtual = std::min(JANELA_MAXIMA, janelaAtual + 1);
        }

        while (pacotesRecebidos.count(ultimoACK + 1))
        {
            ultimoACK++;
        }

        char ackMsg[BUFFER_SIZE];
        sprintf(ackMsg, "ACK %d JANELA %d", ultimoACK, janelaAtual);
        sendto(serverSocket, ackMsg, sizeof(ackMsg), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
        std::cout << "ACK " << ultimoACK << " enviado. Janela: " << janelaAtual << std::endl;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

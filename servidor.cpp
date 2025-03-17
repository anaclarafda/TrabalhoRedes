#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <map>
#include <fstream>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define TOTAL_PACOTES 10000 // Total de pacotes a ser recebido

int main()
{
    WSADATA wsaData;
    SOCKET serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Erro ao inicializar o Winsock." << std::endl;
        return 1;
    }

    // Cria o socket do servidor
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Erro ao criar o socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Prepara o endereço do servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Associa o socket ao endereço do servidor
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Erro ao associar o socket ao endereço." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor aguardando pacotes..." << std::endl;

    // Para armazenar pacotes na ordem correta
    std::map<int, std::string> pacotesRecebidos;
    int ultimoPacoteRecebido = -1;

    // Abrir o arquivo para registrar os pacotes recebidos
    std::ofstream logFile("pacotes_recebidos.txt");
    if (!logFile.is_open())
    {
        std::cerr << "Erro ao abrir o arquivo para gravar os pacotes recebidos!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Loop para receber pacotes até atingir o total
    while (pacotesRecebidos.size() < TOTAL_PACOTES)
    {
        int bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0,
                                     (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR)
        {
            std::cerr << "Erro ao receber dados." << std::endl;
            continue;
        }

        // Extrair número do pacote e dados
        int numeroPacote;
        char dados[BUFFER_SIZE];
        sscanf(buffer, "%d|%s", &numeroPacote, dados);

        // Armazenar o pacote recebido
        pacotesRecebidos[numeroPacote] = dados;

        // Verificar se há pacotes fora de sequência
        if (numeroPacote != ultimoPacoteRecebido + 1)
        {
            std::cerr << "Pacote fora de sequencia detectado. Esperado: "
                      << ultimoPacoteRecebido + 1 << " | Recebido: " << numeroPacote << std::endl;
        }

        // Atualizar o último pacote recebido
        ultimoPacoteRecebido = numeroPacote;

        // Enviar confirmação com o tamanho da janela para o remetente
        std::string resposta = "Pacote " + std::to_string(numeroPacote) + " recebido.";
        sendto(serverSocket, resposta.c_str(), resposta.size(), 0, (struct sockaddr *)&clientAddr, clientAddrLen);

        // Registra o pacote recebido
        std::time_t currentTime = std::time(nullptr);
        logFile << numeroPacote << "," << currentTime << std::endl;
        std::cout << "Pacote " << numeroPacote << " recebido." << std::endl;
    }

    logFile.close();
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

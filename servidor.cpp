#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <map> // Para garantir a ordem dos pacotes
#include <ctime> // Para medir tempo

#pragma comment(lib, "ws2_32.lib")
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erro ao inicializar o Winsock." << std::endl;
        return 1;
    }

    // Cria o socket do servidor
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Erro ao criar o socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Prepara o endereço do servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Associa o socket ao endereço do servidor
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Erro ao associar o socket ao endereço." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor aguardando pacotes..." << std::endl;

    // Para armazenar pacotes na ordem correta
    std::map<int, std::string> pacotesRecebidos;
    int ultimoPacoteRecebido = -1;

    // Loop para receber pacotes
    while (true) {
        int bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, 
                                     (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Erro ao receber dados." << std::endl;
            continue;
        }

        // Extrair número do pacote e dados
        int numeroPacote;
        char dados[BUFFER_SIZE];
        sscanf(buffer, "%d|%s", &numeroPacote, dados);

        // Armazenar o pacote recebido
        pacotesRecebidos[numeroPacote] = dados;

        // Verificar se há pacotes perdidos
        if (numeroPacote != ultimoPacoteRecebido + 1) {
            std::cerr << "Pacote fora de sequência detectado. Esperado: "
                      << ultimoPacoteRecebido + 1 << " | Recebido: " << numeroPacote << std::endl;
        }

        // Atualizar o último pacote recebido
        ultimoPacoteRecebido = numeroPacote;

        // Exibe confirmação
        std::cout << "Pacote " << numeroPacote << " recebido." << std::endl;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

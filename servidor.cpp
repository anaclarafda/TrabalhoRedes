#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET sock;
    sockaddr_in server, client;
    int client_len = sizeof(client);
    char buffer[BUFFER_SIZE];

    // Inicializa a biblioteca Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Erro ao inicializar o Winsock!" << std::endl;
        return 1;
    }

    // Cria o socket UDP
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Erro ao criar o socket!" << std::endl;
        WSACleanup();
        return 1;
    }

    // Prepara o endereço do servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // Associa o socket ao endereço e porta
    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "Erro ao associar o socket!" << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor aguardando pacotes na porta " << SERVER_PORT << "..." << std::endl;

    // Recebe os pacotes do cliente
    while (true) {
        int bytes_received = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client, &client_len);
        if (bytes_received == SOCKET_ERROR) {
            std::cerr << "Erro ao receber pacote!" << std::endl;
            continue;
        }

        // Exibe o conteúdo recebido
        std::cout << "Pacote recebido: " << bytes_received << " bytes" << std::endl;
        std::cout << "Conteúdo do pacote: ";
        for (int i = 0; i < bytes_received; i++) {
            std::cout << buffer[i];  // Imprime os dados do pacote
        }
        std::cout << std::endl;

        // Responde ao cliente
        const char* msg = "Pacote recebido com sucesso!";
        sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&client, client_len);
    }

    // Fecha o socket
    closesocket(sock);
    WSACleanup();
    return 0;
}

#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <cstring>

#define SERVER "127.0.0.1"
#define PORT 8080
#define BUF_SIZE 1024  // Tamanho do buffer

int main() {
    // Inicializa a biblioteca Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Erro ao inicializar o Winsock!" << std::endl;
        return 1;
    }

    // Cria o socket UDP
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Erro ao criar o socket!" << std::endl;
        WSACleanup();
        return 1;
    }

    // Configura o endereço do servidor
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER);

    // Abre o arquivo de dados (teste.bin) para envio
    std::ifstream file("teste.bin", std::ios::binary);
    if (!file) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char buffer[BUF_SIZE];
    int packet_number = 0;

    // Envia os pacotes do arquivo
    while (file.read(buffer, sizeof(buffer))) {
        // Envia o pacote para o servidor
        int bytes_read = file.gcount();  // Tamanho do pacote lido
        if (sendto(sock, buffer, bytes_read, 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
            std::cerr << "Erro ao enviar o pacote!" << std::endl;
            break;
        }

        // Imprime o número do pacote enviado
        std::cout << "Enviando pacote " << ++packet_number << std::endl;
    }

    // Envia os últimos bytes restantes, caso o arquivo não seja múltiplo de BUF_SIZE
    if (file.gcount() > 0) {
        if (sendto(sock, buffer, file.gcount(), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
            std::cerr << "Erro ao enviar o pacote!" << std::endl;
        }
        std::cout << "Enviando pacote " << ++packet_number << std::endl;
    }

    std::cout << "Arquivo enviado com sucesso!" << std::endl;

    // Fecha o arquivo e o socket
    file.close();
    closesocket(sock);
    WSACleanup();

    return 0;
}

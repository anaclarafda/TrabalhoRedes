#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <cstring>
#include <ctime>  // Para medir o tempo

#define SERVER "127.0.0.1"
#define PORT 8080
#define BUF_SIZE 1024  // Tamanho do buffer

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char pacote[BUF_SIZE];
    int totalPacotes = 10000; // Total de pacotes a enviar

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erro ao inicializar o Winsock." << std::endl;
        return 1;
    }

    // Cria o socket do cliente
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Erro ao criar o socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Prepara o endereço do servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER);

    // Abrir o arquivo para registrar os pacotes enviados
    std::ofstream logFile("pacotes_enviados.txt"); // Arquivo para registrar os pacotes enviados
    if (!logFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para gravar os pacotes enviados!" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Loop para enviar os pacotes
    for (int i = 0; i < totalPacotes; i++) {
        // Preenche o pacote com dados
        std::memset(pacote, 0, sizeof(pacote));  // Limpar o buffer
        std::sprintf(pacote, "Pacote %d", i + 1); // Exemplo de conteúdo do pacote

        // Envia o pacote
        sendto(clientSocket, pacote, sizeof(pacote), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        // Registra o envio
        std::time_t currentTime = std::time(nullptr); // Obtém o horário atual
        logFile << i + 1 << "," << currentTime << std::endl; // Registra o número do pacote e o tempo
        std::cout << "Pacote " << i + 1 << " enviado" << std::endl;
    }

    // Fecha o arquivo de log
    logFile.close();

    // Fecha o socket
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

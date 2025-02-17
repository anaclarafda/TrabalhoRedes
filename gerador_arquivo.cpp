#include <iostream>
#include <fstream>
#include <cstdlib>  // Para rand()
#include <ctime>    // Para srand()

#define FILE_SIZE 10485760  // 10MB (10 * 1024 * 1024)

int main() {
    std::ofstream file("teste.bin", std::ios::binary);
    if (!file) {
        std::cerr << "Erro ao criar arquivo!" << std::endl;
        return 1;
    }

    srand(time(0));  // Inicializa o gerador de números aleatórios

    char buffer[1024];  // Buffer de 1KB (1024 bytes)
    for (int i = 0; i < FILE_SIZE / sizeof(buffer); i++) {
        for (int j = 0; j < sizeof(buffer); j++) {
            buffer[j] = rand() % 256;  // Gera um byte aleatório (0-255)
        }
        file.write(buffer, sizeof(buffer));
    }

    file.close();
    std::cout << "Arquivo de teste criado: teste.bin (10MB)" << std::endl;

    return 0;
}

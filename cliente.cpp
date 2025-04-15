#include <iostream>   // Para entrada e saida
#include <fcntl.h>    // Para open() com flags
#include <unistd.h>   // Para write() e close()
#include <cstring>    // Para c_str()

using namespace std;

int main() {
    const char* caminho_fifo = "pipe_requisicao"; // Nome do FIFO

    // Abre o FIFO para escrita
    int fd = open(caminho_fifo, O_WRONLY);
    if (fd == -1) {
        cerr << "Erro ao abrir o FIFO para escrita.\n";
        return 1;
    }

    // Solicita entrada do usuario
    string requisicao;
    cout << "Digite a requisicao (ex: INSERT id=1 nome=lucas | DELETE id=1 | SELECT id=1 | UPDATE id=1 nome=mariana):\n> ";

    getline(cin, requisicao);

    // Escreve a requisicao no FIFO
    write(fd, requisicao.c_str(), requisicao.length()); // Envia a mensagem
    write(fd, "\n", 1); // Envia quebra de linha para facilitar leitura

    close(fd); // Fecha o FIFO
    return 0;
}

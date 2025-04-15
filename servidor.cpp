// Autores Nathanael Dayan Cella e Guilherme Baptista Da Silva
#include "banco.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <sys/stat.h>
using namespace std;

// Vetor que armazena os registros do banco
vector<Registro> bancoDados;

// Mutex para acesso concorrente seguro ao banco
mutex bancoMutex;

// Caminho do FIFO (pipe nomeado)
const char* caminho_fifo = "pipe_requisicao";

// Carrega os dados do arquivo texto para memoria
void carregarBanco(const string& arquivo) {
    bancoDados.clear();
    ifstream in(arquivo);
    string linha;
    while (getline(in, linha)) {
        istringstream ss(linha);
        Registro r;
        ss >> r.id;
        ss.get();
        ss.getline(r.nome, 50);
        bancoDados.push_back(r);
    }
}

// Salva os dados da memoria no arquivo
void salvarBanco(const string& arquivo) {
    ofstream out(arquivo);
    for (auto& r : bancoDados) {
        out << r.id << " " << r.nome << "\n";
    }
}

// Insere novo registro no banco
bool inserirRegistro(int id, const string& nome) {
    lock_guard<mutex> lock(bancoMutex);
    for (auto& r : bancoDados)
        if (r.id == id)
            return false;
    Registro novo;
    novo.id = id;
    strncpy(novo.nome, nome.c_str(), sizeof(novo.nome));
    bancoDados.push_back(novo);
    salvarBanco("banco.txt");
    return true;
}

// Remove registro com id especificado
bool deletarRegistro(int id) {
    lock_guard<mutex> lock(bancoMutex);
    for (auto it = bancoDados.begin(); it != bancoDados.end(); ++it)
        if (it->id == id) {
            bancoDados.erase(it);
            salvarBanco("banco.txt");
            return true;
        }
    return false;
}

// Seleciona um registro com base no id
bool selecionarRegistro(int id, Registro& resultado) {
    lock_guard<mutex> lock(bancoMutex);
    for (auto& r : bancoDados)
        if (r.id == id) {
            resultado = r;
            return true;
        }
    return false;
}

// Atualiza o nome de um registro existente
bool atualizarRegistro(int id, const string& novoNome) {
    lock_guard<mutex> lock(bancoMutex);
    for (auto& r : bancoDados)
        if (r.id == id) {
            strncpy(r.nome, novoNome.c_str(), sizeof(r.nome));
            salvarBanco("banco.txt");
            return true;
        }
    return false;
}

// Processa a string de requisicao e executa a acao correspondente
void processarRequisicao(const string& req) {
    if (req.find("INSERT") != string::npos) {
        int id;
        char nome[50];
        sscanf(req.c_str(), "INSERT id=%d nome=%s", &id, nome);
        if (inserirRegistro(id, nome))
            cout << "[INSERT] Registro inserido.\n";
        else
            cout << "[INSERT] ID ja existe.\n";
    } else if (req.find("DELETE") != string::npos) {
        int id;
        sscanf(req.c_str(), "DELETE id=%d", &id);
        if (deletarRegistro(id))
            cout << "[DELETE] Registro removido.\n";
        else
            cout << "[DELETE] Registro nao encontrado.\n";
    } else if (req.find("SELECT") != string::npos) {
        int id;
        sscanf(req.c_str(), "SELECT id=%d", &id);
        Registro r;
        if (selecionarRegistro(id, r))
            cout << "[SELECT] id=" << r.id << " nome=" << r.nome << "\n";
        else
            cout << "[SELECT] Registro nao encontrado.\n";
    } else if (req.find("UPDATE") != string::npos) {
        int id;
        char nome[50];
        sscanf(req.c_str(), "UPDATE id=%d nome=%s", &id, nome);
        if (atualizarRegistro(id, nome))
            cout << "[UPDATE] Registro atualizado.\n";
        else
            cout << "[UPDATE] Registro nao encontrado.\n";
    } else {
        cout << "[ERRO] Comando invalido.\n";
    }
}

// Encerra o programa e remove o FIFO
void finalizar(int) {
    unlink(caminho_fifo);
    cout << "\nServidor finalizado.\n";
    exit(0);
}

int main() {
    signal(SIGINT, finalizar); // Captura CTRL+C para encerrar corretamente
    carregarBanco("banco.txt");

    // Cria o FIFO se nao existir
    if (mkfifo(caminho_fifo, 0666) == -1 && errno != EEXIST) {
        cerr << "Erro ao criar FIFO.\n";
        return 1;
    }

    cout << "Servidor iniciado. Aguardando requisicoes via FIFO...\n";

    // Loop principal
    while (true) {
        // Abre FIFO para leitura
        int fd = open(caminho_fifo, O_RDONLY);
        if (fd == -1) {
            cerr << "Erro ao abrir FIFO para leitura.\n";
            break;
        }

        char buffer[256] = {0}; // Buffer para leitura
        ssize_t lidos = read(fd, buffer, sizeof(buffer) - 1); // Le a mensagem
        close(fd); // Fecha apos leitura

        if (lidos > 0) {
            string requisicao(buffer); // Converte para string
            thread t(processarRequisicao, requisicao); // Processa em nova thread
            t.detach(); // Detacha para nao bloquear
        }

        this_thread::sleep_for(chrono::milliseconds(200)); // Espera pequena
    }

    return 0;
}

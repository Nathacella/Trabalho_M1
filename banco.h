#ifndef BANCO_H
#define BANCO_H

#include <string>
#include <vector>
#include <mutex>

using namespace std;

struct Registro {
    int id;
    char nome[50];
};

void carregarBanco(const string& arquivo);
void salvarBanco(const string& arquivo);

bool inserirRegistro(int id, const string& nome);
bool deletarRegistro(int id);
bool selecionarRegistro(int id, Registro& resultado);
bool atualizarRegistro(int id, const string& novoNome);

#endif

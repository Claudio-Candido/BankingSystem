#pragma once

#include <stdexcept>
#include <string>

namespace banca {

class ExcecaoBancaria : public std::runtime_error {
public:
    explicit ExcecaoBancaria(const std::string& mensagem)
        : std::runtime_error(mensagem) {}
};

class ExcecaoValidacao : public ExcecaoBancaria {
public:
    explicit ExcecaoValidacao(const std::string& mensagem)
        : ExcecaoBancaria("Erro de validacao: " + mensagem) {}
};

class ExcecaoNaoEncontrado : public ExcecaoBancaria {
public:
    explicit ExcecaoNaoEncontrado(const std::string& mensagem)
        : ExcecaoBancaria("Nao encontrado: " + mensagem) {}
};

class ExcecaoFundosInsuficientes : public ExcecaoBancaria {
public:
    explicit ExcecaoFundosInsuficientes(const std::string& mensagem)
        : ExcecaoBancaria("Fundos insuficientes: " + mensagem) {}
};

class ExcecaoContaBloqueada : public ExcecaoBancaria {
public:
    explicit ExcecaoContaBloqueada(const std::string& mensagem)
        : ExcecaoBancaria("Conta bloqueada: " + mensagem) {}
};

class ExcecaoPersistencia : public ExcecaoBancaria {
public:
    explicit ExcecaoPersistencia(const std::string& mensagem)
        : ExcecaoBancaria("Erro de persistencia: " + mensagem) {}
};

class ExcecaoSessao : public ExcecaoBancaria {
public:
    explicit ExcecaoSessao(const std::string& mensagem)
        : ExcecaoBancaria("Erro de sessao: " + mensagem) {}
};

}  // namespace banca

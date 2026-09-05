#pragma once

#include "banca/comum/Excecoes.hpp"
#include "banca/comum/Tipos.hpp"

#include <cctype>
#include <regex>
#include <string>

namespace banca {
namespace validacao {

inline void exigirNaoVazio(const std::string& valor, const std::string& campo) {
    if (valor.empty()) {
        throw ExcecaoValidacao(campo + " nao pode estar vazio");
    }
}

inline void exigirMontantePositivo(Centavos montante) {
    if (montante <= 0) {
        throw ExcecaoValidacao("O montante deve ser maior que zero");
    }
}

inline void exigirMontanteNaoNegativo(Centavos montante) {
    if (montante < 0) {
        throw ExcecaoValidacao("O montante nao pode ser negativo");
    }
}

inline void exigirNomeValido(const std::string& nome) {
    exigirNaoVazio(nome, "Nome");
    if (nome.size() < 2 || nome.size() > 80) {
        throw ExcecaoValidacao("O nome deve ter entre 2 e 80 caracteres");
    }
}

inline void exigirEmailValido(const std::string& email) {
    exigirNaoVazio(email, "Email");
    // Padrao educativo de demonstracao — nao e RFC-completo.
    static const std::regex padrao(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    if (!std::regex_match(email, padrao)) {
        throw ExcecaoValidacao("Formato de email invalido");
    }
}

inline void exigirTelefoneValido(const std::string& telefone) {
    exigirNaoVazio(telefone, "Telefone");
    int digitos = 0;
    for (char c : telefone) {
        if (std::isdigit(static_cast<unsigned char>(c))) ++digitos;
        else if (c != '+' && c != '-' && c != ' ' && c != '(' && c != ')') {
            throw ExcecaoValidacao("O telefone contem caracteres invalidos");
        }
    }
    if (digitos < 7 || digitos > 15) {
        throw ExcecaoValidacao("O telefone deve conter entre 7 e 15 digitos");
    }
}

/// Analisa entrada monetaria como "100", "100.5", "100.50" para centavos.
inline Centavos analisarDinheiro(const std::string& texto) {
    exigirNaoVazio(texto, "Montante");
    static const std::regex padrao(R"(^\d+(\.\d{1,2})?$)");
    if (!std::regex_match(texto, padrao)) {
        throw ExcecaoValidacao("Formato monetario invalido (esperado ex. 100.50)");
    }
    const auto ponto = texto.find('.');
    if (ponto == std::string::npos) {
        return std::stoll(texto) * 100;
    }
    const auto inteiro = std::stoll(texto.substr(0, ponto));
    auto frac = texto.substr(ponto + 1);
    if (frac.size() == 1) frac.push_back('0');
    return inteiro * 100 + std::stoll(frac);
}

}  // namespace validacao
}  // namespace banca

#pragma once

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace banca {

/// Montante monetário em centavos inteiros para evitar erros de vírgula flutuante.
using Centavos = std::int64_t;

enum class TipoConta {
    Corrente,
    Poupanca,
    Empresa
};

enum class EstadoConta {
    Ativa,
    Bloqueada,
    Encerrada
};

enum class TipoTransacao {
    Deposito,
    Levantamento,
    TransferenciaEntrada,
    TransferenciaSaida,
    Taxa,
    Juro
};

inline std::string paraTexto(TipoConta tipo) {
    switch (tipo) {
        case TipoConta::Corrente: return "Corrente";
        case TipoConta::Poupanca: return "Poupanca";
        case TipoConta::Empresa:  return "Empresa";
    }
    return "Desconhecido";
}

inline std::string paraTexto(EstadoConta estado) {
    switch (estado) {
        case EstadoConta::Ativa:     return "Ativa";
        case EstadoConta::Bloqueada: return "Bloqueada";
        case EstadoConta::Encerrada: return "Encerrada";
    }
    return "Desconhecido";
}

inline std::string paraTexto(TipoTransacao tipo) {
    switch (tipo) {
        case TipoTransacao::Deposito:              return "Deposito";
        case TipoTransacao::Levantamento:          return "Levantamento";
        case TipoTransacao::TransferenciaEntrada:  return "TransferenciaEntrada";
        case TipoTransacao::TransferenciaSaida:    return "TransferenciaSaida";
        case TipoTransacao::Taxa:                  return "Taxa";
        case TipoTransacao::Juro:                  return "Juro";
    }
    return "Desconhecido";
}

inline TipoConta tipoContaDeTexto(const std::string& s) {
    if (s == "Corrente") return TipoConta::Corrente;
    if (s == "Poupanca") return TipoConta::Poupanca;
    if (s == "Empresa")  return TipoConta::Empresa;
    throw std::invalid_argument("TipoConta desconhecido: " + s);
}

inline EstadoConta estadoContaDeTexto(const std::string& s) {
    if (s == "Ativa")     return EstadoConta::Ativa;
    if (s == "Bloqueada") return EstadoConta::Bloqueada;
    if (s == "Encerrada") return EstadoConta::Encerrada;
    throw std::invalid_argument("EstadoConta desconhecido: " + s);
}

inline TipoTransacao tipoTransacaoDeTexto(const std::string& s) {
    if (s == "Deposito")             return TipoTransacao::Deposito;
    if (s == "Levantamento")         return TipoTransacao::Levantamento;
    if (s == "TransferenciaEntrada") return TipoTransacao::TransferenciaEntrada;
    if (s == "TransferenciaSaida")   return TipoTransacao::TransferenciaSaida;
    if (s == "Taxa")                 return TipoTransacao::Taxa;
    if (s == "Juro")                 return TipoTransacao::Juro;
    throw std::invalid_argument("TipoTransacao desconhecido: " + s);
}

/// Formata centavos como "123.45" (sem símbolo de moeda).
inline std::string formatarDinheiro(Centavos centavos) {
    bool neg = centavos < 0;
    if (neg) centavos = -centavos;
    auto inteiro = centavos / 100;
    auto frac = centavos % 100;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%s%lld.%02lld",
                  neg ? "-" : "",
                  static_cast<long long>(inteiro),
                  static_cast<long long>(frac));
    return buf;
}

}  // namespace banca

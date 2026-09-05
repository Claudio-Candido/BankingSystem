#pragma once

#include "banca/comum/Tipos.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <string>
#include <utility>

namespace banca {

class Transacao {
public:
    Transacao() = default;

    Transacao(std::string id,
              std::string idConta,
              TipoTransacao tipo,
              Centavos montante,
              Centavos saldoApos,
              std::string descricao,
              std::string idContaRelacionada = {},
              std::chrono::system_clock::time_point carimboTempo = std::chrono::system_clock::now())
        : id_(std::move(id))
        , idConta_(std::move(idConta))
        , tipo_(tipo)
        , montante_(montante)
        , saldoApos_(saldoApos)
        , descricao_(std::move(descricao))
        , idContaRelacionada_(std::move(idContaRelacionada))
        , carimboTempo_(carimboTempo) {}

    const std::string& id() const { return id_; }
    const std::string& idConta() const { return idConta_; }
    TipoTransacao tipo() const { return tipo_; }
    Centavos montante() const { return montante_; }
    Centavos saldoApos() const { return saldoApos_; }
    const std::string& descricao() const { return descricao_; }
    const std::string& idContaRelacionada() const { return idContaRelacionada_; }
    std::chrono::system_clock::time_point carimboTempo() const { return carimboTempo_; }

    std::string carimboTempoIso() const {
        const auto t = std::chrono::system_clock::to_time_t(carimboTempo_);
        std::tm tm{};
#if defined(_WIN32)
        gmtime_s(&tm, &t);
#else
        gmtime_r(&t, &tm);
#endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
        return buf;
    }

    static std::chrono::system_clock::time_point analisarCarimboTempo(const std::string& iso) {
        std::tm tm{};
        if (std::sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%dZ",
                        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                        &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6) {
            return std::chrono::system_clock::now();
        }
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
#if defined(_WIN32)
        const auto t = _mkgmtime(&tm);
#else
        const auto t = timegm(&tm);
#endif
        return std::chrono::system_clock::from_time_t(t);
    }

private:
    std::string id_;
    std::string idConta_;
    TipoTransacao tipo_{TipoTransacao::Deposito};
    Centavos montante_{0};
    Centavos saldoApos_{0};
    std::string descricao_;
    std::string idContaRelacionada_;
    std::chrono::system_clock::time_point carimboTempo_{std::chrono::system_clock::now()};
};

}  // namespace banca

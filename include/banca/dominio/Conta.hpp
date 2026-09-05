#pragma once

#include "banca/comum/Tipos.hpp"

#include <memory>
#include <string>
#include <utility>

namespace banca {

/// Conta base abstrata — regras polimorficas de taxas e juros por tipo.
class Conta {
public:
    Conta(std::string id,
          std::string nomeTitular,
          std::string email,
          std::string telefone,
          Centavos saldo = 0,
          EstadoConta estado = EstadoConta::Ativa)
        : id_(std::move(id))
        , nomeTitular_(std::move(nomeTitular))
        , email_(std::move(email))
        , telefone_(std::move(telefone))
        , saldo_(saldo)
        , estado_(estado) {}

    virtual ~Conta() = default;

    Conta(const Conta&) = default;
    Conta& operator=(const Conta&) = default;
    Conta(Conta&&) noexcept = default;
    Conta& operator=(Conta&&) noexcept = default;

    const std::string& id() const { return id_; }
    const std::string& nomeTitular() const { return nomeTitular_; }
    const std::string& email() const { return email_; }
    const std::string& telefone() const { return telefone_; }
    Centavos saldo() const { return saldo_; }
    EstadoConta estado() const { return estado_; }

    void definirNomeTitular(std::string nome) { nomeTitular_ = std::move(nome); }
    void definirEmail(std::string email) { email_ = std::move(email); }
    void definirTelefone(std::string telefone) { telefone_ = std::move(telefone); }
    void definirEstado(EstadoConta estado) { estado_ = estado; }
    void definirSaldo(Centavos saldo) { saldo_ = saldo; }

    virtual TipoConta tipo() const = 0;
    virtual Centavos taxaLevantamento(Centavos montante) const = 0;
    virtual Centavos taxaTransferencia(Centavos montante) const = 0;
    virtual double taxaJuroAnual() const = 0;
    virtual Centavos limiteDescoberto() const = 0;
    virtual std::unique_ptr<Conta> clonar() const = 0;

    bool podeDebitar(Centavos montanteComTaxas) const {
        return saldo_ - montanteComTaxas >= -limiteDescoberto();
    }

    void creditar(Centavos montante) { saldo_ += montante; }
    void debitar(Centavos montante) { saldo_ -= montante; }

    bool estaAtiva() const { return estado_ == EstadoConta::Ativa; }
    bool estaBloqueada() const { return estado_ == EstadoConta::Bloqueada; }

protected:
    std::string id_;
    std::string nomeTitular_;
    std::string email_;
    std::string telefone_;
    Centavos saldo_;
    EstadoConta estado_;
};

}  // namespace banca

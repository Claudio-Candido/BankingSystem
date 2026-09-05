#pragma once

#include "banca/dominio/Conta.hpp"

namespace banca {

/// Conta poupanca com juro e sem descoberto; transferencias gratis.
class ContaPoupanca : public Conta {
public:
    using Conta::Conta;

    TipoConta tipo() const override { return TipoConta::Poupanca; }

    Centavos taxaLevantamento(Centavos /*montante*/) const override {
        return 150;  // 1.50
    }

    Centavos taxaTransferencia(Centavos /*montante*/) const override {
        return 0;
    }

    double taxaJuroAnual() const override { return 0.02; }  // 2%

    Centavos limiteDescoberto() const override { return 0; }

    std::unique_ptr<Conta> clonar() const override {
        return std::make_unique<ContaPoupanca>(*this);
    }
};

}  // namespace banca

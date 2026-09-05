#pragma once

#include "banca/dominio/Conta.hpp"

namespace banca {

/// Conta do dia a dia com taxas baixas e descoberto limitado.
class ContaCorrente : public Conta {
public:
    using Conta::Conta;

    TipoConta tipo() const override { return TipoConta::Corrente; }

    Centavos taxaLevantamento(Centavos /*montante*/) const override {
        return 50;  // 0.50
    }

    Centavos taxaTransferencia(Centavos /*montante*/) const override {
        return 100;  // 1.00
    }

    double taxaJuroAnual() const override { return 0.0; }

    Centavos limiteDescoberto() const override { return 20000; }  // 200.00

    std::unique_ptr<Conta> clonar() const override {
        return std::make_unique<ContaCorrente>(*this);
    }
};

}  // namespace banca

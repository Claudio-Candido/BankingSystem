#pragma once

#include "banca/dominio/Conta.hpp"

namespace banca {

/// Conta empresarial com maior descoberto e taxas de transferencia percentuais.
class ContaEmpresa : public Conta {
public:
    using Conta::Conta;

    TipoConta tipo() const override { return TipoConta::Empresa; }

    Centavos taxaLevantamento(Centavos /*montante*/) const override {
        return 0;
    }

    Centavos taxaTransferencia(Centavos montante) const override {
        // 0.1% com minimo de 2.00
        const Centavos pct = montante / 1000;
        return pct < 200 ? 200 : pct;
    }

    double taxaJuroAnual() const override { return 0.005; }

    Centavos limiteDescoberto() const override { return 500000; }  // 5000.00

    std::unique_ptr<Conta> clonar() const override {
        return std::make_unique<ContaEmpresa>(*this);
    }
};

}  // namespace banca

#pragma once

#include "banca/comum/Excecoes.hpp"
#include "banca/dominio/Conta.hpp"
#include "banca/dominio/ContaCorrente.hpp"
#include "banca/dominio/ContaEmpresa.hpp"
#include "banca/dominio/ContaPoupanca.hpp"

#include <memory>
#include <string>
#include <utility>

namespace banca {

class FabricaContas {
public:
    static std::unique_ptr<Conta> criar(TipoConta tipo,
                                        std::string id,
                                        std::string nomeTitular,
                                        std::string email,
                                        std::string telefone,
                                        Centavos saldo = 0,
                                        EstadoConta estado = EstadoConta::Ativa) {
        switch (tipo) {
            case TipoConta::Corrente:
                return std::make_unique<ContaCorrente>(
                    std::move(id), std::move(nomeTitular), std::move(email),
                    std::move(telefone), saldo, estado);
            case TipoConta::Poupanca:
                return std::make_unique<ContaPoupanca>(
                    std::move(id), std::move(nomeTitular), std::move(email),
                    std::move(telefone), saldo, estado);
            case TipoConta::Empresa:
                return std::make_unique<ContaEmpresa>(
                    std::move(id), std::move(nomeTitular), std::move(email),
                    std::move(telefone), saldo, estado);
        }
        throw ExcecaoValidacao("Tipo de conta nao suportado");
    }
};

}  // namespace banca

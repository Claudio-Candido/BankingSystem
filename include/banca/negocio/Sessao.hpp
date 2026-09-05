#pragma once

#include "banca/comum/Excecoes.hpp"

#include <optional>
#include <string>

namespace banca {

/// Sessao de demonstracao em memoria. Nenhuma credencial e guardada ou verificada.
/// Selecionar uma conta abre uma sessao apenas para fluxos educativos da UI.
struct Sessao {
    std::string token;
    std::string idConta;
    std::string nomeTitular;
};

class GestorSessao {
public:
    bool temSessaoAtiva() const { return sessao_.has_value(); }

    const Sessao& atual() const {
        if (!sessao_) throw ExcecaoSessao("Nenhuma sessao ativa — inicie sessao primeiro");
        return *sessao_;
    }

    void abrir(Sessao sessao) { sessao_ = std::move(sessao); }

    void fechar() { sessao_.reset(); }

    const std::string& idConta() const { return atual().idConta; }

private:
    std::optional<Sessao> sessao_;
};

}  // namespace banca

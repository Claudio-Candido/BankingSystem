#pragma once

#include "banca/comum/Excecoes.hpp"
#include "banca/comum/Registador.hpp"
#include "banca/dados/IRepositorio.hpp"
#include "banca/dados/UtilCsv.hpp"
#include "banca/dominio/FabricaContas.hpp"

#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace banca {

/// Persiste contas em CSV. Guarda contas polimorficas via shared_ptr.
class RepositorioContas {
public:
    explicit RepositorioContas(std::string caminhoFicheiro)
        : caminhoFicheiro_(std::move(caminhoFicheiro)) {
        carregar();
    }

    void guardar(std::shared_ptr<Conta> conta) {
        if (!conta) throw ExcecaoPersistencia("Nao e possivel guardar conta nula");
        contas_[conta->id()] = std::move(conta);
        persistir();
    }

    std::shared_ptr<Conta> procurarPorId(const std::string& id) const {
        const auto it = contas_.find(id);
        if (it == contas_.end()) return nullptr;
        return it->second;
    }

    std::vector<std::shared_ptr<Conta>> procurarTodos() const {
        std::vector<std::shared_ptr<Conta>> resultado;
        resultado.reserve(contas_.size());
        for (const auto& [_, conta] : contas_) {
            resultado.push_back(conta);
        }
        return resultado;
    }

    bool remover(const std::string& id) {
        const auto apagado = contas_.erase(id) > 0;
        if (apagado) persistir();
        return apagado;
    }

    void limpar() {
        contas_.clear();
        persistir();
    }

    std::size_t tamanho() const { return contas_.size(); }

    void recarregar() {
        contas_.clear();
        carregar();
    }

private:
    void carregar() {
        std::ifstream entrada(caminhoFicheiro_);
        if (!entrada.is_open()) {
            Registador::instancia().info("Armazenamento de contas nao encontrado; a iniciar vazio: " + caminhoFicheiro_);
            return;
        }
        std::string linha;
        // Ignorar cabecalho se presente
        if (std::getline(entrada, linha)) {
            if (linha.rfind("id,", 0) != 0) {
                analisarLinhaConta(linha);
            }
        }
        while (std::getline(entrada, linha)) {
            if (linha.empty()) continue;
            analisarLinhaConta(linha);
        }
        Registador::instancia().info("Carregadas " + std::to_string(contas_.size()) + " contas");
    }

    void analisarLinhaConta(const std::string& linha) {
        const auto campos = csv::dividirLinha(linha);
        if (campos.size() < 7) {
            Registador::instancia().aviso("A ignorar linha de conta malformada");
            return;
        }
        try {
            std::shared_ptr<Conta> conta = FabricaContas::criar(
                tipoContaDeTexto(campos[1]),
                campos[0],
                campos[2],
                campos[3],
                campos[4],
                std::stoll(campos[5]),
                estadoContaDeTexto(campos[6]));
            contas_[conta->id()] = std::move(conta);
        } catch (const std::exception& ex) {
            Registador::instancia().erro(std::string("Falha ao analisar conta: ") + ex.what());
        }
    }

    void persistir() const {
        std::ofstream saida(caminhoFicheiro_, std::ios::trunc);
        if (!saida.is_open()) {
            throw ExcecaoPersistencia("Nao foi possivel abrir ficheiro de contas para escrita: " + caminhoFicheiro_);
        }
        saida << "id,tipo,nomeTitular,email,telefone,saldoCentavos,estado\n";
        for (const auto& [_, conta] : contas_) {
            saida << csv::juntar({
                conta->id(),
                paraTexto(conta->tipo()),
                conta->nomeTitular(),
                conta->email(),
                conta->telefone(),
                std::to_string(conta->saldo()),
                paraTexto(conta->estado())
            }) << '\n';
        }
    }

    std::string caminhoFicheiro_;
    std::unordered_map<std::string, std::shared_ptr<Conta>> contas_;
};

}  // namespace banca

#pragma once

#include "banca/comum/Excecoes.hpp"
#include "banca/comum/Registador.hpp"
#include "banca/dados/UtilCsv.hpp"
#include "banca/dominio/Transacao.hpp"

#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace banca {

class RepositorioTransacoes {
public:
    explicit RepositorioTransacoes(std::string caminhoFicheiro)
        : caminhoFicheiro_(std::move(caminhoFicheiro)) {
        carregar();
    }

    void guardar(const Transacao& tx) {
        porConta_[tx.idConta()].push_back(tx);
        todas_.push_back(tx);
        persistir();
    }

    std::vector<Transacao> procurarPorConta(const std::string& idConta) const {
        const auto it = porConta_.find(idConta);
        if (it == porConta_.end()) return {};
        auto resultado = it->second;
        std::sort(resultado.begin(), resultado.end(),
                  [](const Transacao& a, const Transacao& b) {
                      return a.carimboTempo() < b.carimboTempo();
                  });
        return resultado;
    }

    std::vector<Transacao> procurarTodos() const { return todas_; }

    void limpar() {
        porConta_.clear();
        todas_.clear();
        persistir();
    }

    std::size_t tamanho() const { return todas_.size(); }

private:
    void carregar() {
        std::ifstream entrada(caminhoFicheiro_);
        if (!entrada.is_open()) {
            Registador::instancia().info("Armazenamento de transacoes nao encontrado; a iniciar vazio: " + caminhoFicheiro_);
            return;
        }
        std::string linha;
        if (std::getline(entrada, linha)) {
            if (linha.rfind("id,", 0) != 0) {
                analisarLinha(linha);
            }
        }
        while (std::getline(entrada, linha)) {
            if (linha.empty()) continue;
            analisarLinha(linha);
        }
        Registador::instancia().info("Carregadas " + std::to_string(todas_.size()) + " transacoes");
    }

    void analisarLinha(const std::string& linha) {
        const auto campos = csv::dividirLinha(linha);
        if (campos.size() < 8) {
            Registador::instancia().aviso("A ignorar linha de transacao malformada");
            return;
        }
        try {
            Transacao tx(
                campos[0],
                campos[1],
                tipoTransacaoDeTexto(campos[2]),
                std::stoll(campos[3]),
                std::stoll(campos[4]),
                campos[5],
                campos[6],
                Transacao::analisarCarimboTempo(campos[7]));
            porConta_[tx.idConta()].push_back(tx);
            todas_.push_back(tx);
        } catch (const std::exception& ex) {
            Registador::instancia().erro(std::string("Falha ao analisar transacao: ") + ex.what());
        }
    }

    void persistir() const {
        std::ofstream saida(caminhoFicheiro_, std::ios::trunc);
        if (!saida.is_open()) {
            throw ExcecaoPersistencia("Nao foi possivel abrir ficheiro de transacoes para escrita: " + caminhoFicheiro_);
        }
        saida << "id,idConta,tipo,montanteCentavos,saldoAposCentavos,descricao,idContaRelacionada,carimboTempo\n";
        for (const auto& tx : todas_) {
            saida << csv::juntar({
                tx.id(),
                tx.idConta(),
                paraTexto(tx.tipo()),
                std::to_string(tx.montante()),
                std::to_string(tx.saldoApos()),
                tx.descricao(),
                tx.idContaRelacionada(),
                tx.carimboTempoIso()
            }) << '\n';
        }
    }

    std::string caminhoFicheiro_;
    std::unordered_map<std::string, std::vector<Transacao>> porConta_;
    std::vector<Transacao> todas_;
};

}  // namespace banca

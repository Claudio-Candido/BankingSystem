#pragma once

#include "banca/comum/Excecoes.hpp"
#include "banca/comum/Registador.hpp"
#include "banca/comum/Validacao.hpp"
#include "banca/dados/RepositorioContas.hpp"
#include "banca/dados/RepositorioTransacoes.hpp"
#include "banca/dominio/FabricaContas.hpp"
#include "banca/negocio/GeradorIds.hpp"
#include "banca/negocio/Sessao.hpp"

#include <algorithm>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace banca {

/// Camada de logica de negocio: orquestra regras de dominio sobre repositorios.
class ServicoBancario {
public:
    ServicoBancario(std::shared_ptr<RepositorioContas> contas,
                    std::shared_ptr<RepositorioTransacoes> transacoes,
                    std::string diretorioDados = "data")
        : contas_(std::move(contas))
        , transacoes_(std::move(transacoes))
        , diretorioDados_(std::move(diretorioDados)) {
        semearIdsContas();
    }

    const std::string& diretorioDados() const { return diretorioDados_; }

    GestorSessao& sessao() { return sessao_; }
    const GestorSessao& sessao() const { return sessao_; }

    // --- Ciclo de vida da conta ---

    std::shared_ptr<Conta> criarConta(TipoConta tipo,
                                      const std::string& nomeTitular,
                                      const std::string& email,
                                      const std::string& telefone,
                                      Centavos depositoInicial = 0) {
        validacao::exigirNomeValido(nomeTitular);
        validacao::exigirEmailValido(email);
        validacao::exigirTelefoneValido(telefone);
        validacao::exigirMontanteNaoNegativo(depositoInicial);

        std::shared_ptr<Conta> conta = FabricaContas::criar(
            tipo, GeradorIds::proximoIdConta(), nomeTitular, email, telefone, 0);

        contas_->guardar(conta);
        Registador::instancia().info("Conta criada " + conta->id() +
                                     " tipo=" + paraTexto(tipo));

        if (depositoInicial > 0) {
            depositar(conta->id(), depositoInicial, "Deposito inicial");
        }
        return contas_->procurarPorId(conta->id());
    }

    /// Login demo: abre sessao apenas pelo numero da conta (sem verificacao de credenciais).
    Sessao iniciarSessao(const std::string& idConta) {
        auto conta = exigirConta(idConta);
        if (conta->estado() == EstadoConta::Encerrada) {
            throw ExcecaoContaBloqueada("A conta esta encerrada");
        }
        Sessao s{GeradorIds::proximoTokenSessao(), conta->id(), conta->nomeTitular()};
        sessao_.abrir(s);
        Registador::instancia().info("Sessao aberta para " + idConta + " token=" + s.token);
        return s;
    }

    void terminarSessao() {
        if (sessao_.temSessaoAtiva()) {
            Registador::instancia().info("Sessao terminada para " + sessao_.idConta());
        }
        sessao_.fechar();
    }

    // --- Operacoes ---

    Centavos obterSaldo(const std::string& idConta) const {
        return exigirConta(idConta)->saldo();
    }

    void depositar(const std::string& idConta, Centavos montante,
                   const std::string& descricao = "Deposito") {
        validacao::exigirMontantePositivo(montante);
        auto conta = exigirContaAtiva(idConta);
        conta->creditar(montante);
        contas_->guardar(conta);
        registar(conta, TipoTransacao::Deposito, montante, descricao);
        Registador::instancia().info("Deposito " + formatarDinheiro(montante) + " em " + idConta);
    }

    void levantar(const std::string& idConta, Centavos montante,
                  const std::string& descricao = "Levantamento") {
        validacao::exigirMontantePositivo(montante);
        auto conta = exigirContaAtiva(idConta);
        const auto taxa = conta->taxaLevantamento(montante);
        const auto total = montante + taxa;
        if (!conta->podeDebitar(total)) {
            throw ExcecaoFundosInsuficientes(
                "Necessario " + formatarDinheiro(total) + " (incl. taxa " + formatarDinheiro(taxa) +
                "), saldo efetivo disponivel " + formatarDinheiro(conta->saldo() + conta->limiteDescoberto()));
        }
        conta->debitar(montante);
        contas_->guardar(conta);
        registar(conta, TipoTransacao::Levantamento, montante, descricao);
        if (taxa > 0) {
            conta->debitar(taxa);
            contas_->guardar(conta);
            registar(conta, TipoTransacao::Taxa, taxa, "Taxa de levantamento");
        }
        Registador::instancia().info("Levantamento " + formatarDinheiro(montante) + " de " + idConta);
    }

    void transferir(const std::string& deId, const std::string& paraId, Centavos montante,
                    const std::string& descricao = "Transferencia") {
        validacao::exigirMontantePositivo(montante);
        if (deId == paraId) {
            throw ExcecaoValidacao("Nao e possivel transferir para a mesma conta");
        }
        auto de = exigirContaAtiva(deId);
        auto para = exigirContaAtiva(paraId);

        const auto taxa = de->taxaTransferencia(montante);
        const auto total = montante + taxa;
        if (!de->podeDebitar(total)) {
            throw ExcecaoFundosInsuficientes(
                "Necessario " + formatarDinheiro(total) + " incluindo taxa de transferencia " + formatarDinheiro(taxa));
        }

        de->debitar(montante);
        para->creditar(montante);
        contas_->guardar(de);
        contas_->guardar(para);

        registar(de, TipoTransacao::TransferenciaSaida, montante, descricao, paraId);
        registar(para, TipoTransacao::TransferenciaEntrada, montante, descricao, deId);

        if (taxa > 0) {
            de->debitar(taxa);
            contas_->guardar(de);
            registar(de, TipoTransacao::Taxa, taxa, "Taxa de transferencia");
        }
        Registador::instancia().info("Transferencia " + formatarDinheiro(montante) + " " + deId + " -> " + paraId);
    }

    std::vector<Transacao> historico(const std::string& idConta) const {
        exigirConta(idConta);
        return transacoes_->procurarPorConta(idConta);
    }

    void bloquearConta(const std::string& idConta) {
        auto conta = exigirConta(idConta);
        if (conta->estado() == EstadoConta::Encerrada) {
            throw ExcecaoValidacao("Contas encerradas nao podem ser bloqueadas");
        }
        conta->definirEstado(EstadoConta::Bloqueada);
        contas_->guardar(conta);
        Registador::instancia().aviso("Conta bloqueada: " + idConta);
    }

    void desbloquearConta(const std::string& idConta) {
        auto conta = exigirConta(idConta);
        if (conta->estado() != EstadoConta::Bloqueada) {
            throw ExcecaoValidacao("A conta nao esta bloqueada");
        }
        conta->definirEstado(EstadoConta::Ativa);
        contas_->guardar(conta);
        Registador::instancia().info("Conta desbloqueada: " + idConta);
    }

    void atualizarDadosConta(const std::string& idConta,
                             const std::string& nomeTitular,
                             const std::string& email,
                             const std::string& telefone) {
        validacao::exigirNomeValido(nomeTitular);
        validacao::exigirEmailValido(email);
        validacao::exigirTelefoneValido(telefone);
        auto conta = exigirConta(idConta);
        if (conta->estado() == EstadoConta::Encerrada) {
            throw ExcecaoValidacao("Nao e possivel atualizar uma conta encerrada");
        }
        conta->definirNomeTitular(nomeTitular);
        conta->definirEmail(email);
        conta->definirTelefone(telefone);
        contas_->guardar(conta);
        Registador::instancia().info("Dados atualizados para " + idConta);
    }

    std::string gerarExtrato(const std::string& idConta,
                             const std::string& caminhoSaida) const {
        auto conta = exigirConta(idConta);
        auto txs = transacoes_->procurarPorConta(idConta);

        std::ofstream saida(caminhoSaida);
        if (!saida.is_open()) {
            throw ExcecaoPersistencia("Nao foi possivel escrever extrato em " + caminhoSaida);
        }

        saida << "========================================\n";
        saida << "       SISTEMA BANCARIO — EXTRATO\n";
        saida << "========================================\n";
        saida << "Conta   : " << conta->id() << '\n';
        saida << "Titular : " << conta->nomeTitular() << '\n';
        saida << "Tipo    : " << paraTexto(conta->tipo()) << '\n';
        saida << "Estado  : " << paraTexto(conta->estado()) << '\n';
        saida << "Saldo   : " << formatarDinheiro(conta->saldo()) << '\n';
        saida << "----------------------------------------\n";
        saida << "Data                 Tipo         Montante   Saldo Apos     Descricao\n";
        saida << "----------------------------------------\n";

        for (const auto& tx : txs) {
            saida << tx.carimboTempoIso() << "  "
                  << preencher(paraTexto(tx.tipo()), 12) << " "
                  << preencher(formatarDinheiro(tx.montante()), 10) << " "
                  << preencher(formatarDinheiro(tx.saldoApos()), 14) << " "
                  << tx.descricao();
            if (!tx.idContaRelacionada().empty()) {
                saida << " [" << tx.idContaRelacionada() << "]";
            }
            saida << '\n';
        }
        saida << "========================================\n";
        saida << "Total de transacoes: " << txs.size() << '\n';

        Registador::instancia().info("Extrato gerado: " + caminhoSaida);
        return caminhoSaida;
    }

    std::vector<std::shared_ptr<Conta>> listarContas() const {
        return contas_->procurarTodos();
    }

    std::shared_ptr<Conta> obterConta(const std::string& idConta) const {
        return exigirConta(idConta);
    }

private:
    static std::string preencher(const std::string& s, std::size_t largura) {
        if (s.size() >= largura) return s;
        return s + std::string(largura - s.size(), ' ');
    }

    std::shared_ptr<Conta> exigirConta(const std::string& idConta) const {
        validacao::exigirNaoVazio(idConta, "ID da conta");
        auto conta = contas_->procurarPorId(idConta);
        if (!conta) throw ExcecaoNaoEncontrado("Conta " + idConta);
        return conta;
    }

    std::shared_ptr<Conta> exigirContaAtiva(const std::string& idConta) const {
        auto conta = exigirConta(idConta);
        if (conta->estaBloqueada()) {
            throw ExcecaoContaBloqueada(idConta);
        }
        if (conta->estado() == EstadoConta::Encerrada) {
            throw ExcecaoContaBloqueada("A conta esta encerrada: " + idConta);
        }
        return conta;
    }

    void registar(const std::shared_ptr<Conta>& conta,
                  TipoTransacao tipo,
                  Centavos montante,
                  const std::string& descricao,
                  const std::string& relacionada = {}) {
        Transacao tx(
            GeradorIds::proximoIdTransacao(),
            conta->id(),
            tipo,
            montante,
            conta->saldo(),
            descricao,
            relacionada);
        transacoes_->guardar(tx);
    }

    void semearIdsContas() {
        int proximo = 1000;
        for (const auto& conta : contas_->procurarTodos()) {
            const auto& id = conta->id();
            if (id.rfind("ACC", 0) == 0) {
                try {
                    proximo = std::max(proximo, std::stoi(id.substr(3)) + 1);
                } catch (...) {
                }
            }
        }
        GeradorIds::semearContadorContas(proximo);
    }

    std::shared_ptr<RepositorioContas> contas_;
    std::shared_ptr<RepositorioTransacoes> transacoes_;
    GestorSessao sessao_;
    std::string diretorioDados_;
};

}  // namespace banca

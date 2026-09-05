#pragma once

#include "banca/comum/Registador.hpp"
#include "banca/negocio/ServicoBancario.hpp"

#include <iostream>
#include <memory>
#include <string>

namespace banca {

class InterfaceConsola {
public:
    explicit InterfaceConsola(std::shared_ptr<ServicoBancario> servico)
        : servico_(std::move(servico)) {}

    void executar() {
        imprimirBanner();
        bool aCorrer = true;
        while (aCorrer) {
            try {
                if (!servico_->sessao().temSessaoAtiva()) {
                    aCorrer = mostrarMenuConvidado();
                } else {
                    aCorrer = mostrarMenuComSessao();
                }
            } catch (const ExcecaoBancaria& ex) {
                std::cout << "\n[ERRO] " << ex.what() << "\n";
                Registador::instancia().erro(ex.what());
            } catch (const std::exception& ex) {
                std::cout << "\n[INESPERADO] " << ex.what() << "\n";
                Registador::instancia().erro(ex.what());
            }
        }
        std::cout << "\nAdeus.\n";
    }

private:
    void imprimirBanner() const {
        std::cout << "\n"
                  << "============================================\n"
                  << "        SISTEMA BANCARIO (Demo)\n"
                  << "============================================\n"
                  << " Projeto educativo — NAO e um banco real.\n"
                  << " Nenhuma palavra-passe ou credencial e guardada.\n"
                  << " Iniciar sessao = selecionar uma conta (demo).\n"
                  << "============================================\n";
    }

    bool mostrarMenuConvidado() {
        std::cout << "\n--- Menu Principal ---\n"
                  << "1. Criar conta\n"
                  << "2. Iniciar sessao (selecionar conta)\n"
                  << "3. Listar contas\n"
                  << "4. Bloquear / desbloquear conta\n"
                  << "0. Sair\n"
                  << "Opcao: ";
        const int opcao = lerInteiro();
        switch (opcao) {
            case 1: tratarCriarConta(); return true;
            case 2: tratarIniciarSessao(); return true;
            case 3: tratarListarContas(); return true;
            case 4: tratarMenuBloqueio(); return true;
            case 0: return false;
            default:
                std::cout << "Opcao invalida.\n";
                return true;
        }
    }

    bool mostrarMenuComSessao() {
        const auto& s = servico_->sessao().atual();
        auto conta = servico_->obterConta(s.idConta);
        std::cout << "\n--- Sessao: " << s.nomeTitular << " (" << s.idConta << ") ---\n"
                  << "Tipo: " << paraTexto(conta->tipo())
                  << " | Estado: " << paraTexto(conta->estado())
                  << " | Saldo: " << formatarDinheiro(conta->saldo()) << "\n"
                  << "1. Depositar\n"
                  << "2. Levantar\n"
                  << "3. Transferir\n"
                  << "4. Saldo\n"
                  << "5. Historico de transacoes\n"
                  << "6. Atualizar dados pessoais\n"
                  << "7. Gerar extrato\n"
                  << "8. Terminar sessao\n"
                  << "0. Sair\n"
                  << "Opcao: ";
        const int opcao = lerInteiro();
        switch (opcao) {
            case 1: tratarDepositar(); return true;
            case 2: tratarLevantar(); return true;
            case 3: tratarTransferir(); return true;
            case 4: tratarSaldo(); return true;
            case 5: tratarHistorico(); return true;
            case 6: tratarAtualizarDados(); return true;
            case 7: tratarExtrato(); return true;
            case 8: servico_->terminarSessao(); std::cout << "Sessao terminada.\n"; return true;
            case 0: servico_->terminarSessao(); return false;
            default:
                std::cout << "Opcao invalida.\n";
                return true;
        }
    }

    void tratarCriarConta() {
        std::cout << "Tipo de conta:\n"
                  << "1. Corrente\n"
                  << "2. Poupanca\n"
                  << "3. Empresa\n"
                  << "Opcao: ";
        const int t = lerInteiro();
        TipoConta tipo = TipoConta::Corrente;
        if (t == 2) tipo = TipoConta::Poupanca;
        else if (t == 3) tipo = TipoConta::Empresa;
        else if (t != 1) {
            std::cout << "Tipo invalido.\n";
            return;
        }

        std::cout << "Nome do titular: ";
        const auto nome = lerLinha();
        std::cout << "Email: ";
        const auto email = lerLinha();
        std::cout << "Telefone: ";
        const auto telefone = lerLinha();
        std::cout << "Deposito inicial (ex. 100.00, ou 0): ";
        const auto montante = validacao::analisarDinheiro(lerLinha());

        auto conta = servico_->criarConta(tipo, nome, email, telefone, montante);
        std::cout << "\nConta criada com sucesso!\n"
                  << "  ID     : " << conta->id() << "\n"
                  << "  Tipo   : " << paraTexto(conta->tipo()) << "\n"
                  << "  Saldo  : " << formatarDinheiro(conta->saldo()) << "\n"
                  << "Use o ID acima para iniciar sessao.\n";
    }

    void tratarIniciarSessao() {
        std::cout << "ID da conta: ";
        const auto id = lerLinha();
        auto sessao = servico_->iniciarSessao(id);
        std::cout << "Sessao aberta. Token: " << sessao.token
                  << " (apenas demo — nao e credencial)\n";
    }

    void tratarListarContas() {
        auto contas = servico_->listarContas();
        if (contas.empty()) {
            std::cout << "Ainda nao existem contas.\n";
            return;
        }
        std::cout << "\nID       Tipo       Estado    Titular                 Saldo\n";
        std::cout << "------------------------------------------------------------------\n";
        for (const auto& a : contas) {
            std::cout << a->id() << "  "
                      << preencher(paraTexto(a->tipo()), 10) << " "
                      << preencher(paraTexto(a->estado()), 9) << " "
                      << preencher(a->nomeTitular(), 22) << " "
                      << formatarDinheiro(a->saldo()) << "\n";
        }
    }

    void tratarMenuBloqueio() {
        std::cout << "1. Bloquear conta\n2. Desbloquear conta\nOpcao: ";
        const int c = lerInteiro();
        std::cout << "ID da conta: ";
        const auto id = lerLinha();
        if (c == 1) {
            servico_->bloquearConta(id);
            std::cout << "Conta bloqueada.\n";
        } else if (c == 2) {
            servico_->desbloquearConta(id);
            std::cout << "Conta desbloqueada.\n";
        } else {
            std::cout << "Opcao invalida.\n";
        }
    }

    void tratarDepositar() {
        std::cout << "Montante: ";
        const auto montante = validacao::analisarDinheiro(lerLinha());
        servico_->depositar(servico_->sessao().idConta(), montante);
        std::cout << "Deposito OK. Novo saldo: "
                  << formatarDinheiro(servico_->obterSaldo(servico_->sessao().idConta())) << "\n";
    }

    void tratarLevantar() {
        std::cout << "Montante: ";
        const auto montante = validacao::analisarDinheiro(lerLinha());
        servico_->levantar(servico_->sessao().idConta(), montante);
        std::cout << "Levantamento OK. Novo saldo: "
                  << formatarDinheiro(servico_->obterSaldo(servico_->sessao().idConta())) << "\n";
    }

    void tratarTransferir() {
        std::cout << "ID da conta de destino: ";
        const auto paraId = lerLinha();
        std::cout << "Montante: ";
        const auto montante = validacao::analisarDinheiro(lerLinha());
        servico_->transferir(servico_->sessao().idConta(), paraId, montante);
        std::cout << "Transferencia OK. Novo saldo: "
                  << formatarDinheiro(servico_->obterSaldo(servico_->sessao().idConta())) << "\n";
    }

    void tratarSaldo() {
        const auto id = servico_->sessao().idConta();
        auto conta = servico_->obterConta(id);
        std::cout << "Saldo: " << formatarDinheiro(conta->saldo()) << "\n"
                  << "Limite de descoberto: " << formatarDinheiro(conta->limiteDescoberto()) << "\n"
                  << "Taxa de juro: " << (conta->taxaJuroAnual() * 100) << "%\n";
    }

    void tratarHistorico() {
        auto txs = servico_->historico(servico_->sessao().idConta());
        if (txs.empty()) {
            std::cout << "Sem transacoes.\n";
            return;
        }
        std::cout << "\nCarimboTempo          Tipo         Montante   Saldo       Descricao\n";
        std::cout << "---------------------------------------------------------------------\n";
        for (const auto& tx : txs) {
            std::cout << tx.carimboTempoIso() << "  "
                      << preencher(paraTexto(tx.tipo()), 12) << " "
                      << preencher(formatarDinheiro(tx.montante()), 10) << " "
                      << preencher(formatarDinheiro(tx.saldoApos()), 10) << " "
                      << tx.descricao() << "\n";
        }
    }

    void tratarAtualizarDados() {
        std::cout << "Novo nome do titular: ";
        const auto nome = lerLinha();
        std::cout << "Novo email: ";
        const auto email = lerLinha();
        std::cout << "Novo telefone: ";
        const auto telefone = lerLinha();
        servico_->atualizarDadosConta(servico_->sessao().idConta(), nome, email, telefone);
        std::cout << "Dados atualizados.\n";
    }

    void tratarExtrato() {
        const auto id = servico_->sessao().idConta();
        const std::string caminho = servico_->diretorioDados() + "/extrato_" + id + ".txt";
        servico_->gerarExtrato(id, caminho);
        std::cout << "Extrato escrito em " << caminho << "\n";
    }

    static std::string preencher(const std::string& s, std::size_t w) {
        if (s.size() >= w) return s.substr(0, w);
        return s + std::string(w - s.size(), ' ');
    }

    static std::string lerLinha() {
        std::string linha;
        std::getline(std::cin, linha);
        // Remover \r final (Windows) e espacos
        while (!linha.empty() && (linha.back() == '\r' || linha.back() == ' ')) linha.pop_back();
        return linha;
    }

    static int lerInteiro() {
        std::string linha = lerLinha();
        try {
            return std::stoi(linha);
        } catch (...) {
            return -1;
        }
    }

    std::shared_ptr<ServicoBancario> servico_;
};

}  // namespace banca

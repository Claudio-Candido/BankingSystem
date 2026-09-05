#include "banca/comum/Validacao.hpp"
#include "banca/dados/RepositorioContas.hpp"
#include "banca/dados/RepositorioTransacoes.hpp"
#include "banca/dados/UtilCsv.hpp"
#include "banca/dominio/FabricaContas.hpp"
#include "banca/negocio/ServicoBancario.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;
using namespace banca;

static int g_falhou = 0;
static int g_passou = 0;

#define VERIFICAR(expr) do { \
    if (expr) { ++g_passou; } \
    else { ++g_falhou; std::cerr << "FALHOU: " << #expr << " em " << __FILE__ << ":" << __LINE__ << "\n"; } \
} while (0)

#define VERIFICAR_LANCA(expr) do { \
    bool lancou = false; \
    try { expr; } catch (...) { lancou = true; } \
    VERIFICAR(lancou); \
} while (0)

std::string diretorioTemp(const std::string& nome) {
    auto caminho = fs::temp_directory_path() / ("banca_teste_" + nome);
    fs::remove_all(caminho);
    fs::create_directories(caminho);
    return caminho.string();
}

void teste_analisar_dinheiro() {
    VERIFICAR(validacao::analisarDinheiro("100") == 10000);
    VERIFICAR(validacao::analisarDinheiro("100.5") == 10050);
    VERIFICAR(validacao::analisarDinheiro("100.50") == 10050);
    VERIFICAR(validacao::analisarDinheiro("0.01") == 1);
    VERIFICAR_LANCA(validacao::analisarDinheiro(""));
    VERIFICAR_LANCA(validacao::analisarDinheiro("-10"));
    VERIFICAR_LANCA(validacao::analisarDinheiro("10.999"));
    VERIFICAR(formatarDinheiro(10050) == "100.50");
    VERIFICAR(formatarDinheiro(-25) == "-0.25");
}

void teste_csv_escapar() {
    VERIFICAR(csv::escapar("simples") == "simples");
    VERIFICAR(csv::escapar("a,b") == "\"a,b\"");
    VERIFICAR(csv::escapar("diz \"ola\"") == "\"diz \"\"ola\"\"\"");
    auto campos = csv::dividirLinha("a,\"b,c\",d");
    VERIFICAR(campos.size() == 3);
    VERIFICAR(campos[1] == "b,c");
}

void teste_polimorfismo_taxas() {
    auto corrente = FabricaContas::criar(TipoConta::Corrente, "A1", "Ana", "a@x.com", "+351911111111");
    auto poupanca = FabricaContas::criar(TipoConta::Poupanca,  "A2", "Bob", "b@x.com", "+351922222222");
    auto empresa  = FabricaContas::criar(TipoConta::Empresa,   "A3", "Co",  "c@x.com", "+351933333333");

    VERIFICAR(corrente->tipo() == TipoConta::Corrente);
    VERIFICAR(poupanca->tipo() == TipoConta::Poupanca);
    VERIFICAR(empresa->tipo() == TipoConta::Empresa);

    VERIFICAR(corrente->taxaLevantamento(10000) == 50);
    VERIFICAR(poupanca->taxaLevantamento(10000) == 150);
    VERIFICAR(empresa->taxaLevantamento(10000) == 0);

    VERIFICAR(corrente->limiteDescoberto() == 20000);
    VERIFICAR(poupanca->limiteDescoberto() == 0);
    VERIFICAR(empresa->taxaTransferencia(1000000) == 1000);  // 0.1% de 10000.00
    VERIFICAR(empresa->taxaTransferencia(10000) == 200);      // minimo 2.00
}

void teste_validacao() {
    VERIFICAR_LANCA(validacao::exigirEmailValido("nao-e-email"));
    validacao::exigirEmailValido("user@example.com");
    VERIFICAR_LANCA(validacao::exigirNomeValido("A"));
    validacao::exigirNomeValido("Maria Silva");
    VERIFICAR_LANCA(validacao::exigirMontantePositivo(0));
    validacao::exigirMontantePositivo(1);
}

void teste_persistencia_roundtrip() {
    const auto dir = diretorioTemp("persist");
    {
        RepositorioContas repo(dir + "/contas.csv");
        std::shared_ptr<Conta> a = FabricaContas::criar(
            TipoConta::Poupanca, "ACC9", "Rita", "rita@ex.com", "912345678", 5000);
        repo.guardar(a);
        VERIFICAR(repo.tamanho() == 1);
    }
    {
        RepositorioContas repo(dir + "/contas.csv");
        VERIFICAR(repo.tamanho() == 1);
        auto a = repo.procurarPorId("ACC9");
        VERIFICAR(a != nullptr);
        VERIFICAR(a->nomeTitular() == "Rita");
        VERIFICAR(a->saldo() == 5000);
        VERIFICAR(a->tipo() == TipoConta::Poupanca);
    }
    fs::remove_all(dir);
}

void teste_fluxo_servico_bancario() {
    const auto dir = diretorioTemp("servico");
    auto contas = std::make_shared<RepositorioContas>(dir + "/contas.csv");
    auto txs = std::make_shared<RepositorioTransacoes>(dir + "/transacoes.csv");
    ServicoBancario svc(contas, txs);

    auto a = svc.criarConta(TipoConta::Corrente, "Alice", "alice@ex.com", "911111111", 100000);
    auto b = svc.criarConta(TipoConta::Poupanca, "Bob", "bob@ex.com", "922222222", 50000);

    VERIFICAR(a->saldo() == 100000);
    VERIFICAR(svc.obterSaldo(b->id()) == 50000);

    svc.depositar(a->id(), 2500);
    VERIFICAR(svc.obterSaldo(a->id()) == 102500);

    svc.levantar(a->id(), 1000);  // + taxa 0.50
    VERIFICAR(svc.obterSaldo(a->id()) == 102500 - 1000 - 50);

    svc.transferir(a->id(), b->id(), 10000);  // taxa transferencia corrente 1.00
    VERIFICAR(svc.obterSaldo(b->id()) == 60000);

    auto hist = svc.historico(a->id());
    VERIFICAR(hist.size() >= 4);  // inicial + deposito + levantar + taxa + transferencia + taxa

    svc.bloquearConta(a->id());
    VERIFICAR_LANCA(svc.depositar(a->id(), 100));
    svc.desbloquearConta(a->id());
    svc.depositar(a->id(), 100);

    svc.atualizarDadosConta(a->id(), "Alice Atualizada", "alice2@ex.com", "933333333");
    VERIFICAR(svc.obterConta(a->id())->nomeTitular() == "Alice Atualizada");

    const auto extrato = dir + "/extrato.txt";
    svc.gerarExtrato(a->id(), extrato);
    VERIFICAR(fs::exists(extrato));

    auto sessao = svc.iniciarSessao(a->id());
    VERIFICAR(svc.sessao().temSessaoAtiva());
    VERIFICAR(sessao.idConta == a->id());
    svc.terminarSessao();
    VERIFICAR(!svc.sessao().temSessaoAtiva());

    VERIFICAR_LANCA(svc.transferir(a->id(), a->id(), 100));
    VERIFICAR_LANCA(svc.levantar(b->id(), 999999999));  // poupanca sem descoberto

    fs::remove_all(dir);
}

void teste_sessao_exige_login() {
    GestorSessao gs;
    VERIFICAR(!gs.temSessaoAtiva());
    VERIFICAR_LANCA(gs.atual());
}

int main() {
    std::cout << "A executar testes do Sistema Bancario...\n";
    teste_analisar_dinheiro();
    teste_csv_escapar();
    teste_polimorfismo_taxas();
    teste_validacao();
    teste_persistencia_roundtrip();
    teste_fluxo_servico_bancario();
    teste_sessao_exige_login();

    std::cout << "Passou: " << g_passou << "  Falhou: " << g_falhou << "\n";
    return g_falhou == 0 ? 0 : 1;
}

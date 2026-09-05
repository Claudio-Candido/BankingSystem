#include "banca/comum/Registador.hpp"
#include "banca/dados/RepositorioContas.hpp"
#include "banca/dados/RepositorioTransacoes.hpp"
#include "banca/negocio/ServicoBancario.hpp"
#include "banca/ui/InterfaceConsola.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        std::string diretorioDados = "data";
        if (argc > 1) {
            diretorioDados = argv[1];
        }

        fs::create_directories(diretorioDados);

        banca::Registador::instancia().definirNivel(banca::NivelRegisto::Info);
        banca::Registador::instancia().definirFicheiroRegisto(diretorioDados + "/banca.log");
        banca::Registador::instancia().info("Sistema Bancario a iniciar");

        auto contas = std::make_shared<banca::RepositorioContas>(diretorioDados + "/contas.csv");
        auto transacoes = std::make_shared<banca::RepositorioTransacoes>(diretorioDados + "/transacoes.csv");
        auto servico = std::make_shared<banca::ServicoBancario>(contas, transacoes, diretorioDados);

        banca::InterfaceConsola ui(servico);
        ui.executar();

        banca::Registador::instancia().info("Sistema Bancario parado");
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal: " << ex.what() << '\n';
        return 1;
    }
}

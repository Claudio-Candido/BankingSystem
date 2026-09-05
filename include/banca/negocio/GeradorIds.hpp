#pragma once

#include <atomic>
#include <chrono>
#include <random>
#include <sstream>
#include <string>

namespace banca {

class GeradorIds {
public:
    static void semearContadorContas(int proximoValor) {
        contadorContas().store(proximoValor);
    }

    static std::string proximoIdConta() {
        std::ostringstream oss;
        oss << "ACC" << contadorContas().fetch_add(1);
        return oss.str();
    }

    static std::string proximoIdTransacao() {
        static std::atomic<int> contador{1};
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::ostringstream oss;
        oss << "TX" << ms << "-" << contador.fetch_add(1);
        return oss.str();
    }

    /// Token de sessao de demonstracao — NAO e uma credencial.
    static std::string proximoTokenSessao() {
        static std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist(100000, 999999);
        return "SESS-" + std::to_string(dist(rng));
    }

private:
    static std::atomic<int>& contadorContas() {
        static std::atomic<int> contador{1000};
        return contador;
    }
};

}  // namespace banca

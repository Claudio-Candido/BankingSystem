#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace banca {

enum class NivelRegisto {
    Depuracao,
    Info,
    Aviso,
    Erro
};

inline std::string paraTexto(NivelRegisto nivel) {
    switch (nivel) {
        case NivelRegisto::Depuracao: return "DEPURACAO";
        case NivelRegisto::Info:      return "INFO";
        case NivelRegisto::Aviso:     return "AVISO";
        case NivelRegisto::Erro:      return "ERRO";
    }
    return "DESCONHECIDO";
}

/// Registador singleton thread-safe para consola e ficheiro opcional.
class Registador {
public:
    static Registador& instancia() {
        static Registador registador;
        return registador;
    }

    void definirNivel(NivelRegisto nivel) { nivel_ = nivel; }
    void definirFicheiroRegisto(const std::string& caminho) {
        std::lock_guard<std::mutex> lock(mutex_);
        ficheiro_ = std::make_unique<std::ofstream>(caminho, std::ios::app);
        if (!ficheiro_->is_open()) {
            ficheiro_.reset();
        }
    }

    void registar(NivelRegisto nivel, const std::string& mensagem) {
        if (nivel < nivel_) return;
        std::lock_guard<std::mutex> lock(mutex_);
        const auto linha = formatar(nivel, mensagem);
        std::cerr << linha << '\n';
        if (ficheiro_ && ficheiro_->is_open()) {
            *ficheiro_ << linha << '\n';
            ficheiro_->flush();
        }
    }

    void depuracao(const std::string& msg) { registar(NivelRegisto::Depuracao, msg); }
    void info(const std::string& msg)      { registar(NivelRegisto::Info, msg); }
    void aviso(const std::string& msg)     { registar(NivelRegisto::Aviso, msg); }
    void erro(const std::string& msg)      { registar(NivelRegisto::Erro, msg); }

private:
    Registador() = default;

    std::string formatar(NivelRegisto nivel, const std::string& mensagem) const {
        using relogio = std::chrono::system_clock;
        const auto agora = relogio::now();
        const auto t = relogio::to_time_t(agora);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
            << " [" << paraTexto(nivel) << "] " << mensagem;
        return oss.str();
    }

    NivelRegisto nivel_ = NivelRegisto::Info;
    std::unique_ptr<std::ofstream> ficheiro_;
    std::mutex mutex_;
};

}  // namespace banca

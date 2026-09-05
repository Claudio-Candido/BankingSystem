#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace banca {
namespace csv {

inline std::string escapar(const std::string& valor) {
    bool precisaAspas = valor.find_first_of(",\"\n\r") != std::string::npos;
    if (!precisaAspas) return valor;
    std::string saida = "\"";
    for (char c : valor) {
        if (c == '"') saida += "\"\"";
        else saida += c;
    }
    saida += '"';
    return saida;
}

inline std::vector<std::string> dividirLinha(const std::string& linha) {
    std::vector<std::string> campos;
    std::string campo;
    bool emAspas = false;
    for (size_t i = 0; i < linha.size(); ++i) {
        char c = linha[i];
        if (emAspas) {
            if (c == '"') {
                if (i + 1 < linha.size() && linha[i + 1] == '"') {
                    campo += '"';
                    ++i;
                } else {
                    emAspas = false;
                }
            } else {
                campo += c;
            }
        } else {
            if (c == '"') {
                emAspas = true;
            } else if (c == ',') {
                campos.push_back(campo);
                campo.clear();
            } else {
                campo += c;
            }
        }
    }
    campos.push_back(campo);
    return campos;
}

inline std::string juntar(const std::vector<std::string>& campos) {
    std::ostringstream oss;
    for (size_t i = 0; i < campos.size(); ++i) {
        if (i) oss << ',';
        oss << escapar(campos[i]);
    }
    return oss.str();
}

}  // namespace csv
}  // namespace banca

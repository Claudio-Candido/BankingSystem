#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace banca {

/// Interface generica de repositorio (template justificado como contrato CRUD reutilizavel).
template <typename T, typename Id = std::string>
class IRepositorio {
public:
    virtual ~IRepositorio() = default;

    virtual void guardar(const T& entidade) = 0;
    virtual std::optional<T> procurarPorId(const Id& id) const = 0;
    virtual std::vector<T> procurarTodos() const = 0;
    virtual bool remover(const Id& id) = 0;
    virtual void limpar() = 0;
};

}  // namespace banca

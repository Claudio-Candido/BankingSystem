#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace banking {

/// Generic repository interface (template justified by reusable CRUD contract).
template <typename T, typename Id = std::string>
class IRepository {
public:
    virtual ~IRepository() = default;

    virtual void save(const T& entity) = 0;
    virtual std::optional<T> findById(const Id& id) const = 0;
    virtual std::vector<T> findAll() const = 0;
    virtual bool remove(const Id& id) = 0;
    virtual void clear() = 0;
};

}  // namespace banking

#pragma once

#include "banking/common/Exceptions.hpp"
#include "banking/common/Logger.hpp"
#include "banking/data/CsvUtil.hpp"
#include "banking/data/IRepository.hpp"
#include "banking/domain/AccountFactory.hpp"

#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace banking {

/// Persists accounts to CSV. Holds polymorphic accounts via shared_ptr.
class AccountRepository {
public:
    explicit AccountRepository(std::string filePath)
        : filePath_(std::move(filePath)) {
        load();
    }

    void save(std::shared_ptr<Account> account) {
        if (!account) throw PersistenceException("Cannot save null account");
        accounts_[account->id()] = std::move(account);
        persist();
    }

    std::shared_ptr<Account> findById(const std::string& id) const {
        const auto it = accounts_.find(id);
        if (it == accounts_.end()) return nullptr;
        return it->second;
    }

    std::vector<std::shared_ptr<Account>> findAll() const {
        std::vector<std::shared_ptr<Account>> result;
        result.reserve(accounts_.size());
        for (const auto& [_, account] : accounts_) {
            result.push_back(account);
        }
        return result;
    }

    bool remove(const std::string& id) {
        const auto erased = accounts_.erase(id) > 0;
        if (erased) persist();
        return erased;
    }

    void clear() {
        accounts_.clear();
        persist();
    }

    std::size_t size() const { return accounts_.size(); }

    void reload() {
        accounts_.clear();
        load();
    }

private:
    void load() {
        std::ifstream in(filePath_);
        if (!in.is_open()) {
            Logger::instance().info("Account store not found; starting empty: " + filePath_);
            return;
        }
        std::string line;
        // Skip header if present
        if (std::getline(in, line)) {
            if (line.rfind("id,", 0) != 0) {
                parseAccountLine(line);
            }
        }
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            parseAccountLine(line);
        }
        Logger::instance().info("Loaded " + std::to_string(accounts_.size()) + " accounts");
    }

    void parseAccountLine(const std::string& line) {
        const auto fields = csv::splitLine(line);
        if (fields.size() < 7) {
            Logger::instance().warn("Skipping malformed account line");
            return;
        }
        try {
            std::shared_ptr<Account> account = AccountFactory::create(
                accountTypeFromString(fields[1]),
                fields[0],
                fields[2],
                fields[3],
                fields[4],
                std::stoll(fields[5]),
                accountStatusFromString(fields[6]));
            accounts_[account->id()] = std::move(account);
        } catch (const std::exception& ex) {
            Logger::instance().error(std::string("Failed to parse account: ") + ex.what());
        }
    }

    void persist() const {
        std::ofstream out(filePath_, std::ios::trunc);
        if (!out.is_open()) {
            throw PersistenceException("Cannot open account file for writing: " + filePath_);
        }
        out << "id,type,holderName,email,phone,balanceCents,status\n";
        for (const auto& [_, account] : accounts_) {
            out << csv::join({
                account->id(),
                toString(account->type()),
                account->holderName(),
                account->email(),
                account->phone(),
                std::to_string(account->balance()),
                toString(account->status())
            }) << '\n';
        }
    }

    std::string filePath_;
    std::unordered_map<std::string, std::shared_ptr<Account>> accounts_;
};

}  // namespace banking

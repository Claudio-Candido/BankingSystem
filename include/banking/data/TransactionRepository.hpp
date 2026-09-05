#pragma once

#include "banking/common/Exceptions.hpp"
#include "banking/common/Logger.hpp"
#include "banking/data/CsvUtil.hpp"
#include "banking/domain/Transaction.hpp"

#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace banking {

class TransactionRepository {
public:
    explicit TransactionRepository(std::string filePath)
        : filePath_(std::move(filePath)) {
        load();
    }

    void save(const Transaction& tx) {
        byAccount_[tx.accountId()].push_back(tx);
        all_.push_back(tx);
        persist();
    }

    std::vector<Transaction> findByAccount(const std::string& accountId) const {
        const auto it = byAccount_.find(accountId);
        if (it == byAccount_.end()) return {};
        auto result = it->second;
        std::sort(result.begin(), result.end(),
                  [](const Transaction& a, const Transaction& b) {
                      return a.timestamp() < b.timestamp();
                  });
        return result;
    }

    std::vector<Transaction> findAll() const { return all_; }

    void clear() {
        byAccount_.clear();
        all_.clear();
        persist();
    }

    std::size_t size() const { return all_.size(); }

private:
    void load() {
        std::ifstream in(filePath_);
        if (!in.is_open()) {
            Logger::instance().info("Transaction store not found; starting empty: " + filePath_);
            return;
        }
        std::string line;
        if (std::getline(in, line)) {
            if (line.rfind("id,", 0) != 0) {
                parseLine(line);
            }
        }
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            parseLine(line);
        }
        Logger::instance().info("Loaded " + std::to_string(all_.size()) + " transactions");
    }

    void parseLine(const std::string& line) {
        const auto fields = csv::splitLine(line);
        if (fields.size() < 8) {
            Logger::instance().warn("Skipping malformed transaction line");
            return;
        }
        try {
            Transaction tx(
                fields[0],
                fields[1],
                transactionTypeFromString(fields[2]),
                std::stoll(fields[3]),
                std::stoll(fields[4]),
                fields[5],
                fields[6],
                Transaction::parseTimestamp(fields[7]));
            byAccount_[tx.accountId()].push_back(tx);
            all_.push_back(tx);
        } catch (const std::exception& ex) {
            Logger::instance().error(std::string("Failed to parse transaction: ") + ex.what());
        }
    }

    void persist() const {
        std::ofstream out(filePath_, std::ios::trunc);
        if (!out.is_open()) {
            throw PersistenceException("Cannot open transaction file for writing: " + filePath_);
        }
        out << "id,accountId,type,amountCents,balanceAfterCents,description,relatedAccountId,timestamp\n";
        for (const auto& tx : all_) {
            out << csv::join({
                tx.id(),
                tx.accountId(),
                toString(tx.type()),
                std::to_string(tx.amount()),
                std::to_string(tx.balanceAfter()),
                tx.description(),
                tx.relatedAccountId(),
                tx.timestampIso()
            }) << '\n';
        }
    }

    std::string filePath_;
    std::unordered_map<std::string, std::vector<Transaction>> byAccount_;
    std::vector<Transaction> all_;
};

}  // namespace banking

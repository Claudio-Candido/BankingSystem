#pragma once

#include "banking/common/Types.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <string>
#include <utility>

namespace banking {

class Transaction {
public:
    Transaction() = default;

    Transaction(std::string id,
                std::string accountId,
                TransactionType type,
                MoneyCents amount,
                MoneyCents balanceAfter,
                std::string description,
                std::string relatedAccountId = {},
                std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now())
        : id_(std::move(id))
        , accountId_(std::move(accountId))
        , type_(type)
        , amount_(amount)
        , balanceAfter_(balanceAfter)
        , description_(std::move(description))
        , relatedAccountId_(std::move(relatedAccountId))
        , timestamp_(timestamp) {}

    const std::string& id() const { return id_; }
    const std::string& accountId() const { return accountId_; }
    TransactionType type() const { return type_; }
    MoneyCents amount() const { return amount_; }
    MoneyCents balanceAfter() const { return balanceAfter_; }
    const std::string& description() const { return description_; }
    const std::string& relatedAccountId() const { return relatedAccountId_; }
    std::chrono::system_clock::time_point timestamp() const { return timestamp_; }

    std::string timestampIso() const {
        const auto t = std::chrono::system_clock::to_time_t(timestamp_);
        std::tm tm{};
#if defined(_WIN32)
        gmtime_s(&tm, &t);
#else
        gmtime_r(&t, &tm);
#endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
        return buf;
    }

    static std::chrono::system_clock::time_point parseTimestamp(const std::string& iso) {
        std::tm tm{};
        if (std::sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%dZ",
                        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                        &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6) {
            return std::chrono::system_clock::now();
        }
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
#if defined(_WIN32)
        const auto t = _mkgmtime(&tm);
#else
        const auto t = timegm(&tm);
#endif
        return std::chrono::system_clock::from_time_t(t);
    }

private:
    std::string id_;
    std::string accountId_;
    TransactionType type_{TransactionType::Deposit};
    MoneyCents amount_{0};
    MoneyCents balanceAfter_{0};
    std::string description_;
    std::string relatedAccountId_;
    std::chrono::system_clock::time_point timestamp_{std::chrono::system_clock::now()};
};

}  // namespace banking

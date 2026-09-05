#pragma once

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace banking {

/// Monetary amount stored as integer cents to avoid floating-point errors.
using MoneyCents = std::int64_t;

enum class AccountType {
    Checking,
    Savings,
    Business
};

enum class AccountStatus {
    Active,
    Blocked,
    Closed
};

enum class TransactionType {
    Deposit,
    Withdrawal,
    TransferIn,
    TransferOut,
    Fee,
    Interest
};

inline std::string toString(AccountType type) {
    switch (type) {
        case AccountType::Checking: return "Checking";
        case AccountType::Savings:  return "Savings";
        case AccountType::Business: return "Business";
    }
    return "Unknown";
}

inline std::string toString(AccountStatus status) {
    switch (status) {
        case AccountStatus::Active:  return "Active";
        case AccountStatus::Blocked: return "Blocked";
        case AccountStatus::Closed:  return "Closed";
    }
    return "Unknown";
}

inline std::string toString(TransactionType type) {
    switch (type) {
        case TransactionType::Deposit:      return "Deposit";
        case TransactionType::Withdrawal:   return "Withdrawal";
        case TransactionType::TransferIn:   return "TransferIn";
        case TransactionType::TransferOut:  return "TransferOut";
        case TransactionType::Fee:          return "Fee";
        case TransactionType::Interest:     return "Interest";
    }
    return "Unknown";
}

inline AccountType accountTypeFromString(const std::string& s) {
    if (s == "Checking") return AccountType::Checking;
    if (s == "Savings")  return AccountType::Savings;
    if (s == "Business") return AccountType::Business;
    throw std::invalid_argument("Unknown AccountType: " + s);
}

inline AccountStatus accountStatusFromString(const std::string& s) {
    if (s == "Active")  return AccountStatus::Active;
    if (s == "Blocked") return AccountStatus::Blocked;
    if (s == "Closed")  return AccountStatus::Closed;
    throw std::invalid_argument("Unknown AccountStatus: " + s);
}

inline TransactionType transactionTypeFromString(const std::string& s) {
    if (s == "Deposit")     return TransactionType::Deposit;
    if (s == "Withdrawal")  return TransactionType::Withdrawal;
    if (s == "TransferIn")  return TransactionType::TransferIn;
    if (s == "TransferOut") return TransactionType::TransferOut;
    if (s == "Fee")         return TransactionType::Fee;
    if (s == "Interest")    return TransactionType::Interest;
    throw std::invalid_argument("Unknown TransactionType: " + s);
}

/// Format cents as "123.45" (no currency symbol).
inline std::string formatMoney(MoneyCents cents) {
    bool neg = cents < 0;
    if (neg) cents = -cents;
    auto whole = cents / 100;
    auto frac = cents % 100;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%s%lld.%02lld",
                  neg ? "-" : "",
                  static_cast<long long>(whole),
                  static_cast<long long>(frac));
    return buf;
}

}  // namespace banking

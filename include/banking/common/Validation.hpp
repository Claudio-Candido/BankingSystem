#pragma once

#include "banking/common/Exceptions.hpp"
#include "banking/common/Types.hpp"

#include <cctype>
#include <regex>
#include <string>

namespace banking {
namespace validation {

inline void requireNonEmpty(const std::string& value, const std::string& field) {
    if (value.empty()) {
        throw ValidationException(field + " must not be empty");
    }
}

inline void requirePositiveAmount(MoneyCents amount) {
    if (amount <= 0) {
        throw ValidationException("Amount must be greater than zero");
    }
}

inline void requireNonNegativeAmount(MoneyCents amount) {
    if (amount < 0) {
        throw ValidationException("Amount must not be negative");
    }
}

inline void requireValidName(const std::string& name) {
    requireNonEmpty(name, "Name");
    if (name.size() < 2 || name.size() > 80) {
        throw ValidationException("Name length must be between 2 and 80 characters");
    }
}

inline void requireValidEmail(const std::string& email) {
    requireNonEmpty(email, "Email");
    // Educational demo pattern — not RFC-complete.
    static const std::regex pattern(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    if (!std::regex_match(email, pattern)) {
        throw ValidationException("Invalid email format");
    }
}

inline void requireValidPhone(const std::string& phone) {
    requireNonEmpty(phone, "Phone");
    int digits = 0;
    for (char c : phone) {
        if (std::isdigit(static_cast<unsigned char>(c))) ++digits;
        else if (c != '+' && c != '-' && c != ' ' && c != '(' && c != ')') {
            throw ValidationException("Phone contains invalid characters");
        }
    }
    if (digits < 7 || digits > 15) {
        throw ValidationException("Phone must contain 7 to 15 digits");
    }
}

/// Parse user money input like "100", "100.5", "100.50" into cents.
inline MoneyCents parseMoney(const std::string& text) {
    requireNonEmpty(text, "Amount");
    static const std::regex pattern(R"(^\d+(\.\d{1,2})?$)");
    if (!std::regex_match(text, pattern)) {
        throw ValidationException("Invalid money format (expected e.g. 100.50)");
    }
    const auto dot = text.find('.');
    if (dot == std::string::npos) {
        return std::stoll(text) * 100;
    }
    const auto whole = std::stoll(text.substr(0, dot));
    auto frac = text.substr(dot + 1);
    if (frac.size() == 1) frac.push_back('0');
    return whole * 100 + std::stoll(frac);
}

}  // namespace validation
}  // namespace banking

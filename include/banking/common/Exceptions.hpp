#pragma once

#include <stdexcept>
#include <string>

namespace banking {

class BankingException : public std::runtime_error {
public:
    explicit BankingException(const std::string& message)
        : std::runtime_error(message) {}
};

class ValidationException : public BankingException {
public:
    explicit ValidationException(const std::string& message)
        : BankingException("Validation error: " + message) {}
};

class NotFoundException : public BankingException {
public:
    explicit NotFoundException(const std::string& message)
        : BankingException("Not found: " + message) {}
};

class InsufficientFundsException : public BankingException {
public:
    explicit InsufficientFundsException(const std::string& message)
        : BankingException("Insufficient funds: " + message) {}
};

class AccountBlockedException : public BankingException {
public:
    explicit AccountBlockedException(const std::string& message)
        : BankingException("Account blocked: " + message) {}
};

class PersistenceException : public BankingException {
public:
    explicit PersistenceException(const std::string& message)
        : BankingException("Persistence error: " + message) {}
};

class SessionException : public BankingException {
public:
    explicit SessionException(const std::string& message)
        : BankingException("Session error: " + message) {}
};

}  // namespace banking

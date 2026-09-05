#pragma once

#include "banking/common/Types.hpp"

#include <memory>
#include <string>
#include <utility>

namespace banking {

/// Abstract base account — polymorphic fee and interest rules per type.
class Account {
public:
    Account(std::string id,
            std::string holderName,
            std::string email,
            std::string phone,
            MoneyCents balance = 0,
            AccountStatus status = AccountStatus::Active)
        : id_(std::move(id))
        , holderName_(std::move(holderName))
        , email_(std::move(email))
        , phone_(std::move(phone))
        , balance_(balance)
        , status_(status) {}

    virtual ~Account() = default;

    Account(const Account&) = default;
    Account& operator=(const Account&) = default;
    Account(Account&&) noexcept = default;
    Account& operator=(Account&&) noexcept = default;

    const std::string& id() const { return id_; }
    const std::string& holderName() const { return holderName_; }
    const std::string& email() const { return email_; }
    const std::string& phone() const { return phone_; }
    MoneyCents balance() const { return balance_; }
    AccountStatus status() const { return status_; }

    void setHolderName(std::string name) { holderName_ = std::move(name); }
    void setEmail(std::string email) { email_ = std::move(email); }
    void setPhone(std::string phone) { phone_ = std::move(phone); }
    void setStatus(AccountStatus status) { status_ = status; }
    void setBalance(MoneyCents balance) { balance_ = balance; }

    virtual AccountType type() const = 0;
    virtual MoneyCents withdrawalFee(MoneyCents amount) const = 0;
    virtual MoneyCents transferFee(MoneyCents amount) const = 0;
    virtual double annualInterestRate() const = 0;
    virtual MoneyCents overdraftLimit() const = 0;
    virtual std::unique_ptr<Account> clone() const = 0;

    bool canDebit(MoneyCents amountPlusFees) const {
        return balance_ - amountPlusFees >= -overdraftLimit();
    }

    void credit(MoneyCents amount) { balance_ += amount; }
    void debit(MoneyCents amount) { balance_ -= amount; }

    bool isActive() const { return status_ == AccountStatus::Active; }
    bool isBlocked() const { return status_ == AccountStatus::Blocked; }

protected:
    std::string id_;
    std::string holderName_;
    std::string email_;
    std::string phone_;
    MoneyCents balance_;
    AccountStatus status_;
};

}  // namespace banking

#pragma once

#include "banking/domain/Account.hpp"

namespace banking {

/// Savings account with interest and no overdraft; free transfers.
class SavingsAccount : public Account {
public:
    using Account::Account;

    AccountType type() const override { return AccountType::Savings; }

    MoneyCents withdrawalFee(MoneyCents /*amount*/) const override {
        return 150;  // 1.50
    }

    MoneyCents transferFee(MoneyCents /*amount*/) const override {
        return 0;
    }

    double annualInterestRate() const override { return 0.02; }  // 2%

    MoneyCents overdraftLimit() const override { return 0; }

    std::unique_ptr<Account> clone() const override {
        return std::make_unique<SavingsAccount>(*this);
    }
};

}  // namespace banking

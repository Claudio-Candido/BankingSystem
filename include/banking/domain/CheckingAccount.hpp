#pragma once

#include "banking/domain/Account.hpp"

namespace banking {

/// Everyday account with small fees and limited overdraft.
class CheckingAccount : public Account {
public:
    using Account::Account;

    AccountType type() const override { return AccountType::Checking; }

    MoneyCents withdrawalFee(MoneyCents /*amount*/) const override {
        return 50;  // 0.50
    }

    MoneyCents transferFee(MoneyCents /*amount*/) const override {
        return 100;  // 1.00
    }

    double annualInterestRate() const override { return 0.0; }

    MoneyCents overdraftLimit() const override { return 20000; }  // 200.00

    std::unique_ptr<Account> clone() const override {
        return std::make_unique<CheckingAccount>(*this);
    }
};

}  // namespace banking

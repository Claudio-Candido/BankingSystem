#pragma once

#include "banking/domain/Account.hpp"

namespace banking {

/// Business account with higher overdraft and percentage-based transfer fees.
class BusinessAccount : public Account {
public:
    using Account::Account;

    AccountType type() const override { return AccountType::Business; }

    MoneyCents withdrawalFee(MoneyCents /*amount*/) const override {
        return 0;
    }

    MoneyCents transferFee(MoneyCents amount) const override {
        // 0.1% with a minimum of 2.00
        const MoneyCents pct = amount / 1000;
        return pct < 200 ? 200 : pct;
    }

    double annualInterestRate() const override { return 0.005; }

    MoneyCents overdraftLimit() const override { return 500000; }  // 5000.00

    std::unique_ptr<Account> clone() const override {
        return std::make_unique<BusinessAccount>(*this);
    }
};

}  // namespace banking

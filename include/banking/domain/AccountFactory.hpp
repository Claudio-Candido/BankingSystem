#pragma once

#include "banking/common/Exceptions.hpp"
#include "banking/domain/Account.hpp"
#include "banking/domain/BusinessAccount.hpp"
#include "banking/domain/CheckingAccount.hpp"
#include "banking/domain/SavingsAccount.hpp"

#include <memory>
#include <string>
#include <utility>

namespace banking {

class AccountFactory {
public:
    static std::unique_ptr<Account> create(AccountType type,
                                           std::string id,
                                           std::string holderName,
                                           std::string email,
                                           std::string phone,
                                           MoneyCents balance = 0,
                                           AccountStatus status = AccountStatus::Active) {
        switch (type) {
            case AccountType::Checking:
                return std::make_unique<CheckingAccount>(
                    std::move(id), std::move(holderName), std::move(email),
                    std::move(phone), balance, status);
            case AccountType::Savings:
                return std::make_unique<SavingsAccount>(
                    std::move(id), std::move(holderName), std::move(email),
                    std::move(phone), balance, status);
            case AccountType::Business:
                return std::make_unique<BusinessAccount>(
                    std::move(id), std::move(holderName), std::move(email),
                    std::move(phone), balance, status);
        }
        throw ValidationException("Unsupported account type");
    }
};

}  // namespace banking

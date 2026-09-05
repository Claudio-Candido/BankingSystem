#pragma once

#include "banking/business/IdGenerator.hpp"
#include "banking/business/Session.hpp"
#include "banking/common/Exceptions.hpp"
#include "banking/common/Logger.hpp"
#include "banking/common/Validation.hpp"
#include "banking/data/AccountRepository.hpp"
#include "banking/data/TransactionRepository.hpp"
#include "banking/domain/AccountFactory.hpp"

#include <algorithm>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace banking {

/// Business Logic layer: orchestrates domain rules over repositories.
class BankingService {
public:
    BankingService(std::shared_ptr<AccountRepository> accounts,
                   std::shared_ptr<TransactionRepository> transactions,
                   std::string dataDir = "data")
        : accounts_(std::move(accounts))
        , transactions_(std::move(transactions))
        , dataDir_(std::move(dataDir)) {
        seedAccountIds();
    }

    const std::string& dataDir() const { return dataDir_; }

    SessionManager& session() { return session_; }
    const SessionManager& session() const { return session_; }

    // --- Account lifecycle ---

    std::shared_ptr<Account> createAccount(AccountType type,
                                           const std::string& holderName,
                                           const std::string& email,
                                           const std::string& phone,
                                           MoneyCents initialDeposit = 0) {
        validation::requireValidName(holderName);
        validation::requireValidEmail(email);
        validation::requireValidPhone(phone);
        validation::requireNonNegativeAmount(initialDeposit);

        std::shared_ptr<Account> account = AccountFactory::create(
            type, IdGenerator::nextAccountId(), holderName, email, phone, 0);

        accounts_->save(account);
        Logger::instance().info("Created account " + account->id() +
                                " type=" + toString(type));

        if (initialDeposit > 0) {
            deposit(account->id(), initialDeposit, "Initial deposit");
        }
        return accounts_->findById(account->id());
    }

    /// Demo login: open a session by account number only (no credential check).
    Session login(const std::string& accountId) {
        auto account = requireAccount(accountId);
        if (account->status() == AccountStatus::Closed) {
            throw AccountBlockedException("Account is closed");
        }
        Session s{IdGenerator::nextSessionToken(), account->id(), account->holderName()};
        session_.open(s);
        Logger::instance().info("Session opened for " + accountId + " token=" + s.token);
        return s;
    }

    void logout() {
        if (session_.isLoggedIn()) {
            Logger::instance().info("Session closed for " + session_.accountId());
        }
        session_.close();
    }

    // --- Operations ---

    MoneyCents getBalance(const std::string& accountId) const {
        return requireAccount(accountId)->balance();
    }

    void deposit(const std::string& accountId, MoneyCents amount,
                 const std::string& description = "Deposit") {
        validation::requirePositiveAmount(amount);
        auto account = requireActiveAccount(accountId);
        account->credit(amount);
        accounts_->save(account);
        record(account, TransactionType::Deposit, amount, description);
        Logger::instance().info("Deposit " + formatMoney(amount) + " to " + accountId);
    }

    void withdraw(const std::string& accountId, MoneyCents amount,
                  const std::string& description = "Withdrawal") {
        validation::requirePositiveAmount(amount);
        auto account = requireActiveAccount(accountId);
        const auto fee = account->withdrawalFee(amount);
        const auto total = amount + fee;
        if (!account->canDebit(total)) {
            throw InsufficientFundsException(
                "Need " + formatMoney(total) + " (incl. fee " + formatMoney(fee) +
                "), available effective balance " + formatMoney(account->balance() + account->overdraftLimit()));
        }
        account->debit(amount);
        accounts_->save(account);
        record(account, TransactionType::Withdrawal, amount, description);
        if (fee > 0) {
            account->debit(fee);
            accounts_->save(account);
            record(account, TransactionType::Fee, fee, "Withdrawal fee");
        }
        Logger::instance().info("Withdrawal " + formatMoney(amount) + " from " + accountId);
    }

    void transfer(const std::string& fromId, const std::string& toId, MoneyCents amount,
                  const std::string& description = "Transfer") {
        validation::requirePositiveAmount(amount);
        if (fromId == toId) {
            throw ValidationException("Cannot transfer to the same account");
        }
        auto from = requireActiveAccount(fromId);
        auto to = requireActiveAccount(toId);

        const auto fee = from->transferFee(amount);
        const auto total = amount + fee;
        if (!from->canDebit(total)) {
            throw InsufficientFundsException(
                "Need " + formatMoney(total) + " including transfer fee " + formatMoney(fee));
        }

        from->debit(amount);
        to->credit(amount);
        accounts_->save(from);
        accounts_->save(to);

        record(from, TransactionType::TransferOut, amount, description, toId);
        record(to, TransactionType::TransferIn, amount, description, fromId);

        if (fee > 0) {
            from->debit(fee);
            accounts_->save(from);
            record(from, TransactionType::Fee, fee, "Transfer fee");
        }
        Logger::instance().info("Transfer " + formatMoney(amount) + " " + fromId + " -> " + toId);
    }

    std::vector<Transaction> history(const std::string& accountId) const {
        requireAccount(accountId);
        return transactions_->findByAccount(accountId);
    }

    void blockAccount(const std::string& accountId) {
        auto account = requireAccount(accountId);
        if (account->status() == AccountStatus::Closed) {
            throw ValidationException("Closed accounts cannot be blocked");
        }
        account->setStatus(AccountStatus::Blocked);
        accounts_->save(account);
        Logger::instance().warn("Account blocked: " + accountId);
    }

    void unblockAccount(const std::string& accountId) {
        auto account = requireAccount(accountId);
        if (account->status() != AccountStatus::Blocked) {
            throw ValidationException("Account is not blocked");
        }
        account->setStatus(AccountStatus::Active);
        accounts_->save(account);
        Logger::instance().info("Account unblocked: " + accountId);
    }

    void updateAccountDetails(const std::string& accountId,
                              const std::string& holderName,
                              const std::string& email,
                              const std::string& phone) {
        validation::requireValidName(holderName);
        validation::requireValidEmail(email);
        validation::requireValidPhone(phone);
        auto account = requireAccount(accountId);
        if (account->status() == AccountStatus::Closed) {
            throw ValidationException("Cannot update a closed account");
        }
        account->setHolderName(holderName);
        account->setEmail(email);
        account->setPhone(phone);
        accounts_->save(account);
        Logger::instance().info("Updated details for " + accountId);
    }

    std::string generateStatement(const std::string& accountId,
                                  const std::string& outputPath) const {
        auto account = requireAccount(accountId);
        auto txs = transactions_->findByAccount(accountId);

        std::ofstream out(outputPath);
        if (!out.is_open()) {
            throw PersistenceException("Cannot write statement to " + outputPath);
        }

        out << "========================================\n";
        out << "       BANKING SYSTEM — STATEMENT\n";
        out << "========================================\n";
        out << "Account : " << account->id() << '\n';
        out << "Holder  : " << account->holderName() << '\n';
        out << "Type    : " << toString(account->type()) << '\n';
        out << "Status  : " << toString(account->status()) << '\n';
        out << "Balance : " << formatMoney(account->balance()) << '\n';
        out << "----------------------------------------\n";
        out << "Date                 Type         Amount     Balance After  Description\n";
        out << "----------------------------------------\n";

        for (const auto& tx : txs) {
            out << tx.timestampIso() << "  "
                << pad(toString(tx.type()), 12) << " "
                << pad(formatMoney(tx.amount()), 10) << " "
                << pad(formatMoney(tx.balanceAfter()), 14) << " "
                << tx.description();
            if (!tx.relatedAccountId().empty()) {
                out << " [" << tx.relatedAccountId() << "]";
            }
            out << '\n';
        }
        out << "========================================\n";
        out << "Total transactions: " << txs.size() << '\n';

        Logger::instance().info("Statement generated: " + outputPath);
        return outputPath;
    }

    std::vector<std::shared_ptr<Account>> listAccounts() const {
        return accounts_->findAll();
    }

    std::shared_ptr<Account> getAccount(const std::string& accountId) const {
        return requireAccount(accountId);
    }

private:
    static std::string pad(const std::string& s, std::size_t width) {
        if (s.size() >= width) return s;
        return s + std::string(width - s.size(), ' ');
    }

    std::shared_ptr<Account> requireAccount(const std::string& accountId) const {
        validation::requireNonEmpty(accountId, "Account ID");
        auto account = accounts_->findById(accountId);
        if (!account) throw NotFoundException("Account " + accountId);
        return account;
    }

    std::shared_ptr<Account> requireActiveAccount(const std::string& accountId) const {
        auto account = requireAccount(accountId);
        if (account->isBlocked()) {
            throw AccountBlockedException(accountId);
        }
        if (account->status() == AccountStatus::Closed) {
            throw AccountBlockedException("Account is closed: " + accountId);
        }
        return account;
    }

    void record(const std::shared_ptr<Account>& account,
                TransactionType type,
                MoneyCents amount,
                const std::string& description,
                const std::string& related = {}) {
        Transaction tx(
            IdGenerator::nextTransactionId(),
            account->id(),
            type,
            amount,
            account->balance(),
            description,
            related);
        transactions_->save(tx);
    }

    void seedAccountIds() {
        int next = 1000;
        for (const auto& account : accounts_->findAll()) {
            const auto& id = account->id();
            if (id.rfind("ACC", 0) == 0) {
                try {
                    next = std::max(next, std::stoi(id.substr(3)) + 1);
                } catch (...) {
                }
            }
        }
        IdGenerator::seedAccountCounter(next);
    }

    std::shared_ptr<AccountRepository> accounts_;
    std::shared_ptr<TransactionRepository> transactions_;
    SessionManager session_;
    std::string dataDir_;
};

}  // namespace banking

#pragma once

#include "banking/business/BankingService.hpp"
#include "banking/common/Logger.hpp"

#include <iostream>
#include <memory>
#include <string>

namespace banking {

class ConsoleUI {
public:
    explicit ConsoleUI(std::shared_ptr<BankingService> service)
        : service_(std::move(service)) {}

    void run() {
        printBanner();
        bool running = true;
        while (running) {
            try {
                if (!service_->session().isLoggedIn()) {
                    running = showGuestMenu();
                } else {
                    running = showLoggedInMenu();
                }
            } catch (const BankingException& ex) {
                std::cout << "\n[ERROR] " << ex.what() << "\n";
                Logger::instance().error(ex.what());
            } catch (const std::exception& ex) {
                std::cout << "\n[UNEXPECTED] " << ex.what() << "\n";
                Logger::instance().error(ex.what());
            }
        }
        std::cout << "\nGoodbye.\n";
    }

private:
    void printBanner() const {
        std::cout << "\n"
                  << "============================================\n"
                  << "          BANKING SYSTEM (Demo)\n"
                  << "============================================\n"
                  << " Educational project — NOT a real bank.\n"
                  << " No passwords or credentials are stored.\n"
                  << " Login = select an account for a demo session.\n"
                  << "============================================\n";
    }

    bool showGuestMenu() {
        std::cout << "\n--- Main Menu ---\n"
                  << "1. Create account\n"
                  << "2. Login (select account)\n"
                  << "3. List accounts\n"
                  << "4. Block / unblock account\n"
                  << "0. Exit\n"
                  << "Choice: ";
        const int choice = readInt();
        switch (choice) {
            case 1: handleCreateAccount(); return true;
            case 2: handleLogin(); return true;
            case 3: handleListAccounts(); return true;
            case 4: handleBlockMenu(); return true;
            case 0: return false;
            default:
                std::cout << "Invalid option.\n";
                return true;
        }
    }

    bool showLoggedInMenu() {
        const auto& s = service_->session().current();
        auto account = service_->getAccount(s.accountId);
        std::cout << "\n--- Session: " << s.holderName << " (" << s.accountId << ") ---\n"
                  << "Type: " << toString(account->type())
                  << " | Status: " << toString(account->status())
                  << " | Balance: " << formatMoney(account->balance()) << "\n"
                  << "1. Deposit\n"
                  << "2. Withdraw\n"
                  << "3. Transfer\n"
                  << "4. Balance\n"
                  << "5. Transaction history\n"
                  << "6. Update personal data\n"
                  << "7. Generate statement\n"
                  << "8. Logout\n"
                  << "0. Exit\n"
                  << "Choice: ";
        const int choice = readInt();
        switch (choice) {
            case 1: handleDeposit(); return true;
            case 2: handleWithdraw(); return true;
            case 3: handleTransfer(); return true;
            case 4: handleBalance(); return true;
            case 5: handleHistory(); return true;
            case 6: handleUpdateDetails(); return true;
            case 7: handleStatement(); return true;
            case 8: service_->logout(); std::cout << "Logged out.\n"; return true;
            case 0: service_->logout(); return false;
            default:
                std::cout << "Invalid option.\n";
                return true;
        }
    }

    void handleCreateAccount() {
        std::cout << "Account type:\n"
                  << "1. Checking\n"
                  << "2. Savings\n"
                  << "3. Business\n"
                  << "Choice: ";
        const int t = readInt();
        AccountType type = AccountType::Checking;
        if (t == 2) type = AccountType::Savings;
        else if (t == 3) type = AccountType::Business;
        else if (t != 1) {
            std::cout << "Invalid type.\n";
            return;
        }

        std::cout << "Holder name: ";
        const auto name = readLine();
        std::cout << "Email: ";
        const auto email = readLine();
        std::cout << "Phone: ";
        const auto phone = readLine();
        std::cout << "Initial deposit (e.g. 100.00, or 0): ";
        const auto amount = validation::parseMoney(readLine());

        auto account = service_->createAccount(type, name, email, phone, amount);
        std::cout << "\nAccount created successfully!\n"
                  << "  ID     : " << account->id() << "\n"
                  << "  Type   : " << toString(account->type()) << "\n"
                  << "  Balance: " << formatMoney(account->balance()) << "\n"
                  << "Use the ID above to login.\n";
    }

    void handleLogin() {
        std::cout << "Account ID: ";
        const auto id = readLine();
        auto session = service_->login(id);
        std::cout << "Session opened. Token: " << session.token
                  << " (demo only — not a credential)\n";
    }

    void handleListAccounts() {
        auto accounts = service_->listAccounts();
        if (accounts.empty()) {
            std::cout << "No accounts yet.\n";
            return;
        }
        std::cout << "\nID       Type       Status    Holder                  Balance\n";
        std::cout << "------------------------------------------------------------------\n";
        for (const auto& a : accounts) {
            std::cout << a->id() << "  "
                      << pad(toString(a->type()), 10) << " "
                      << pad(toString(a->status()), 9) << " "
                      << pad(a->holderName(), 22) << " "
                      << formatMoney(a->balance()) << "\n";
        }
    }

    void handleBlockMenu() {
        std::cout << "1. Block account\n2. Unblock account\nChoice: ";
        const int c = readInt();
        std::cout << "Account ID: ";
        const auto id = readLine();
        if (c == 1) {
            service_->blockAccount(id);
            std::cout << "Account blocked.\n";
        } else if (c == 2) {
            service_->unblockAccount(id);
            std::cout << "Account unblocked.\n";
        } else {
            std::cout << "Invalid option.\n";
        }
    }

    void handleDeposit() {
        std::cout << "Amount: ";
        const auto amount = validation::parseMoney(readLine());
        service_->deposit(service_->session().accountId(), amount);
        std::cout << "Deposit OK. New balance: "
                  << formatMoney(service_->getBalance(service_->session().accountId())) << "\n";
    }

    void handleWithdraw() {
        std::cout << "Amount: ";
        const auto amount = validation::parseMoney(readLine());
        service_->withdraw(service_->session().accountId(), amount);
        std::cout << "Withdrawal OK. New balance: "
                  << formatMoney(service_->getBalance(service_->session().accountId())) << "\n";
    }

    void handleTransfer() {
        std::cout << "Destination account ID: ";
        const auto toId = readLine();
        std::cout << "Amount: ";
        const auto amount = validation::parseMoney(readLine());
        service_->transfer(service_->session().accountId(), toId, amount);
        std::cout << "Transfer OK. New balance: "
                  << formatMoney(service_->getBalance(service_->session().accountId())) << "\n";
    }

    void handleBalance() {
        const auto id = service_->session().accountId();
        auto account = service_->getAccount(id);
        std::cout << "Balance: " << formatMoney(account->balance()) << "\n"
                  << "Overdraft limit: " << formatMoney(account->overdraftLimit()) << "\n"
                  << "Interest rate: " << (account->annualInterestRate() * 100) << "%\n";
    }

    void handleHistory() {
        auto txs = service_->history(service_->session().accountId());
        if (txs.empty()) {
            std::cout << "No transactions.\n";
            return;
        }
        std::cout << "\nTimestamp             Type         Amount     Balance     Description\n";
        std::cout << "---------------------------------------------------------------------\n";
        for (const auto& tx : txs) {
            std::cout << tx.timestampIso() << "  "
                      << pad(toString(tx.type()), 12) << " "
                      << pad(formatMoney(tx.amount()), 10) << " "
                      << pad(formatMoney(tx.balanceAfter()), 10) << " "
                      << tx.description() << "\n";
        }
    }

    void handleUpdateDetails() {
        std::cout << "New holder name: ";
        const auto name = readLine();
        std::cout << "New email: ";
        const auto email = readLine();
        std::cout << "New phone: ";
        const auto phone = readLine();
        service_->updateAccountDetails(service_->session().accountId(), name, email, phone);
        std::cout << "Details updated.\n";
    }

    void handleStatement() {
        const auto id = service_->session().accountId();
        const std::string path = service_->dataDir() + "/statement_" + id + ".txt";
        service_->generateStatement(id, path);
        std::cout << "Statement written to " << path << "\n";
    }

    static std::string pad(const std::string& s, std::size_t w) {
        if (s.size() >= w) return s.substr(0, w);
        return s + std::string(w - s.size(), ' ');
    }

    static std::string readLine() {
        std::string line;
        std::getline(std::cin, line);
        // Trim trailing \r (Windows) and spaces
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
        return line;
    }

    static int readInt() {
        std::string line = readLine();
        try {
            return std::stoi(line);
        } catch (...) {
            return -1;
        }
    }

    std::shared_ptr<BankingService> service_;
};

}  // namespace banking

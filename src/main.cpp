#include "banking/business/BankingService.hpp"
#include "banking/common/Logger.hpp"
#include "banking/data/AccountRepository.hpp"
#include "banking/data/TransactionRepository.hpp"
#include "banking/ui/ConsoleUI.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        std::string dataDir = "data";
        if (argc > 1) {
            dataDir = argv[1];
        }

        fs::create_directories(dataDir);

        banking::Logger::instance().setLevel(banking::LogLevel::Info);
        banking::Logger::instance().setLogFile(dataDir + "/banking.log");
        banking::Logger::instance().info("Banking System starting");

        auto accounts = std::make_shared<banking::AccountRepository>(dataDir + "/accounts.csv");
        auto transactions = std::make_shared<banking::TransactionRepository>(dataDir + "/transactions.csv");
        auto service = std::make_shared<banking::BankingService>(accounts, transactions, dataDir);

        banking::ConsoleUI ui(service);
        ui.run();

        banking::Logger::instance().info("Banking System stopped");
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal: " << ex.what() << '\n';
        return 1;
    }
}

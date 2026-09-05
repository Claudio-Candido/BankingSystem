#include "banking/business/BankingService.hpp"
#include "banking/common/Validation.hpp"
#include "banking/data/AccountRepository.hpp"
#include "banking/data/CsvUtil.hpp"
#include "banking/data/TransactionRepository.hpp"
#include "banking/domain/AccountFactory.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;
using namespace banking;

static int g_failed = 0;
static int g_passed = 0;

#define CHECK(expr) do { \
    if (expr) { ++g_passed; } \
    else { ++g_failed; std::cerr << "FAIL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << "\n"; } \
} while (0)

#define CHECK_THROWS(expr) do { \
    bool threw = false; \
    try { expr; } catch (...) { threw = true; } \
    CHECK(threw); \
} while (0)

std::string tempDir(const std::string& name) {
    auto path = fs::temp_directory_path() / ("banking_test_" + name);
    fs::remove_all(path);
    fs::create_directories(path);
    return path.string();
}

void test_money_parse() {
    CHECK(validation::parseMoney("100") == 10000);
    CHECK(validation::parseMoney("100.5") == 10050);
    CHECK(validation::parseMoney("100.50") == 10050);
    CHECK(validation::parseMoney("0.01") == 1);
    CHECK_THROWS(validation::parseMoney(""));
    CHECK_THROWS(validation::parseMoney("-10"));
    CHECK_THROWS(validation::parseMoney("10.999"));
    CHECK(formatMoney(10050) == "100.50");
    CHECK(formatMoney(-25) == "-0.25");
}

void test_csv_escape() {
    CHECK(csv::escape("plain") == "plain");
    CHECK(csv::escape("a,b") == "\"a,b\"");
    CHECK(csv::escape("say \"hi\"") == "\"say \"\"hi\"\"\"");
    auto fields = csv::splitLine("a,\"b,c\",d");
    CHECK(fields.size() == 3);
    CHECK(fields[1] == "b,c");
}

void test_polymorphism_fees() {
    auto checking = AccountFactory::create(AccountType::Checking, "A1", "Ana", "a@x.com", "+351911111111");
    auto savings  = AccountFactory::create(AccountType::Savings,  "A2", "Bob", "b@x.com", "+351922222222");
    auto business = AccountFactory::create(AccountType::Business, "A3", "Co",  "c@x.com", "+351933333333");

    CHECK(checking->type() == AccountType::Checking);
    CHECK(savings->type() == AccountType::Savings);
    CHECK(business->type() == AccountType::Business);

    CHECK(checking->withdrawalFee(10000) == 50);
    CHECK(savings->withdrawalFee(10000) == 150);
    CHECK(business->withdrawalFee(10000) == 0);

    CHECK(checking->overdraftLimit() == 20000);
    CHECK(savings->overdraftLimit() == 0);
    CHECK(business->transferFee(1000000) == 1000);  // 0.1% of 10000.00
    CHECK(business->transferFee(10000) == 200);      // minimum 2.00
}

void test_validation() {
    CHECK_THROWS(validation::requireValidEmail("not-an-email"));
    validation::requireValidEmail("user@example.com");
    CHECK_THROWS(validation::requireValidName("A"));
    validation::requireValidName("Maria Silva");
    CHECK_THROWS(validation::requirePositiveAmount(0));
    validation::requirePositiveAmount(1);
}

void test_persistence_roundtrip() {
    const auto dir = tempDir("persist");
    {
        AccountRepository repo(dir + "/accounts.csv");
        std::shared_ptr<Account> a = AccountFactory::create(
            AccountType::Savings, "ACC9", "Rita", "rita@ex.com", "912345678", 5000);
        repo.save(a);
        CHECK(repo.size() == 1);
    }
    {
        AccountRepository repo(dir + "/accounts.csv");
        CHECK(repo.size() == 1);
        auto a = repo.findById("ACC9");
        CHECK(a != nullptr);
        CHECK(a->holderName() == "Rita");
        CHECK(a->balance() == 5000);
        CHECK(a->type() == AccountType::Savings);
    }
    fs::remove_all(dir);
}

void test_banking_service_flow() {
    const auto dir = tempDir("service");
    auto accounts = std::make_shared<AccountRepository>(dir + "/accounts.csv");
    auto txs = std::make_shared<TransactionRepository>(dir + "/transactions.csv");
    BankingService svc(accounts, txs);

    auto a = svc.createAccount(AccountType::Checking, "Alice", "alice@ex.com", "911111111", 100000);
    auto b = svc.createAccount(AccountType::Savings, "Bob", "bob@ex.com", "922222222", 50000);

    CHECK(a->balance() == 100000);
    CHECK(svc.getBalance(b->id()) == 50000);

    svc.deposit(a->id(), 2500);
    CHECK(svc.getBalance(a->id()) == 102500);

    svc.withdraw(a->id(), 1000);  // + fee 0.50
    CHECK(svc.getBalance(a->id()) == 102500 - 1000 - 50);

    svc.transfer(a->id(), b->id(), 10000);  // checking transfer fee 1.00
    CHECK(svc.getBalance(b->id()) == 60000);

    auto hist = svc.history(a->id());
    CHECK(hist.size() >= 4);  // initial + deposit + withdraw + fee + transfer out + fee

    svc.blockAccount(a->id());
    CHECK_THROWS(svc.deposit(a->id(), 100));
    svc.unblockAccount(a->id());
    svc.deposit(a->id(), 100);

    svc.updateAccountDetails(a->id(), "Alice Updated", "alice2@ex.com", "933333333");
    CHECK(svc.getAccount(a->id())->holderName() == "Alice Updated");

    const auto statement = dir + "/stmt.txt";
    svc.generateStatement(a->id(), statement);
    CHECK(fs::exists(statement));

    auto session = svc.login(a->id());
    CHECK(svc.session().isLoggedIn());
    CHECK(session.accountId == a->id());
    svc.logout();
    CHECK(!svc.session().isLoggedIn());

    CHECK_THROWS(svc.transfer(a->id(), a->id(), 100));
    CHECK_THROWS(svc.withdraw(b->id(), 999999999));  // savings no overdraft

    fs::remove_all(dir);
}

void test_session_requires_login() {
    SessionManager sm;
    CHECK(!sm.isLoggedIn());
    CHECK_THROWS(sm.current());
}

int main() {
    std::cout << "Running Banking System tests...\n";
    test_money_parse();
    test_csv_escape();
    test_polymorphism_fees();
    test_validation();
    test_persistence_roundtrip();
    test_banking_service_flow();
    test_session_requires_login();

    std::cout << "Passed: " << g_passed << "  Failed: " << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}

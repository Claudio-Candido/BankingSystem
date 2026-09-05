#pragma once

#include <atomic>
#include <chrono>
#include <random>
#include <sstream>
#include <string>

namespace banking {

class IdGenerator {
public:
    static void seedAccountCounter(int nextValue) {
        accountCounter().store(nextValue);
    }

    static std::string nextAccountId() {
        std::ostringstream oss;
        oss << "ACC" << accountCounter().fetch_add(1);
        return oss.str();
    }

    static std::string nextTransactionId() {
        static std::atomic<int> counter{1};
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::ostringstream oss;
        oss << "TX" << ms << "-" << counter.fetch_add(1);
        return oss.str();
    }

    /// Demo session token — NOT a credential.
    static std::string nextSessionToken() {
        static std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist(100000, 999999);
        return "SESS-" + std::to_string(dist(rng));
    }

private:
    static std::atomic<int>& accountCounter() {
        static std::atomic<int> counter{1000};
        return counter;
    }
};

}  // namespace banking

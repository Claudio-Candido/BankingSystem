#pragma once

#include "banking/common/Exceptions.hpp"

#include <optional>
#include <string>

namespace banking {

/// In-memory demo session. No credentials are stored or verified cryptographically.
/// Selecting an account opens a session for educational UI flows only.
struct Session {
    std::string token;
    std::string accountId;
    std::string holderName;
};

class SessionManager {
public:
    bool isLoggedIn() const { return session_.has_value(); }

    const Session& current() const {
        if (!session_) throw SessionException("No active session — please login first");
        return *session_;
    }

    void open(Session session) { session_ = std::move(session); }

    void close() { session_.reset(); }

    const std::string& accountId() const { return current().accountId; }

private:
    std::optional<Session> session_;
};

}  // namespace banking

#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace banking {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

inline std::string toString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
    }
    return "UNKNOWN";
}

/// Thread-safe singleton logger writing to console and optional file.
class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void setLevel(LogLevel level) { level_ = level; }
    void setLogFile(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        file_ = std::make_unique<std::ofstream>(path, std::ios::app);
        if (!file_->is_open()) {
            file_.reset();
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (level < level_) return;
        std::lock_guard<std::mutex> lock(mutex_);
        const auto line = format(level, message);
        std::cerr << line << '\n';
        if (file_ && file_->is_open()) {
            *file_ << line << '\n';
            file_->flush();
        }
    }

    void debug(const std::string& msg) { log(LogLevel::Debug, msg); }
    void info(const std::string& msg)  { log(LogLevel::Info, msg); }
    void warn(const std::string& msg)  { log(LogLevel::Warning, msg); }
    void error(const std::string& msg) { log(LogLevel::Error, msg); }

private:
    Logger() = default;

    std::string format(LogLevel level, const std::string& message) const {
        using clock = std::chrono::system_clock;
        const auto now = clock::now();
        const auto t = clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
            << " [" << toString(level) << "] " << message;
        return oss.str();
    }

    LogLevel level_ = LogLevel::Info;
    std::unique_ptr<std::ofstream> file_;
    std::mutex mutex_;
};

}  // namespace banking

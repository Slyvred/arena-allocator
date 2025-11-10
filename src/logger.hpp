#pragma once

#include <ostream>
#include <iostream>
#include <ctime>
#include <utility>

class Logger {
public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    friend std::ostream& operator<<(std::ostream& os, const Logger::LogLevel& level) {
        switch (level) {
            case Logger::LogLevel::DEBUG:   os << "DEBUG"; break;
            case Logger::LogLevel::INFO:    os << "INFO"; break;
            case Logger::LogLevel::WARNING: os << "WARNING"; break;
            case Logger::LogLevel::ERROR:   os << "ERROR"; break;
            default:      os << "UNKNOWN"; break;
        }
        return os;
    }

private:
    LogLevel logLevel_;
    const char* format_;

public:
    Logger(LogLevel log_level, const char* format) : logLevel_(log_level), format_(format) {}

    void log(LogLevel level, const char* msg) {
        if (level < this->logLevel_) return;

        this->printPrefix();
        std::cout << msg << "\n";
    }

    /* printf format support */
    template<typename... Args>
    void log(LogLevel level, const char* msg, Args&&... args) {
        if (level < this->logLevel_) return;

        char buffer[1024];
        std::snprintf(buffer, sizeof(buffer), msg, std::forward<Args>(args)...);

        this->printPrefix();
        std::cout << buffer << "\n";
    }

    LogLevel getLevel() {
        return this->logLevel_;
    }

    const char* getFormat() {
        return this->format_;
    }

    void setLevel(LogLevel level) {
        this->logLevel_ = level;
    }

    void setFormat(const char* format) {
        this->format_ = format;
    }

private:
    void printPrefix() {
        time_t timestamp = time(NULL);
        struct tm datetime = *localtime(&timestamp);
        char time[50];
        std::strftime(time, 50, this->format_, &datetime);
        std::cout << "[" << this->logLevel_ << " - " << time << "] ";
    }
};

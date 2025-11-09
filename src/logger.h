#pragma once

#include <ostream>
#include <string>
#include <iostream>
#include <ctime>
#include <utility>

enum LOG_LEVEL{
  DEBUG,
  INFO,
  WARNING,
  ERROR
};

inline std::ostream& operator<<(std::ostream& os, const LOG_LEVEL& level) {
    switch (level) {
        case DEBUG:   os << "DEBUG"; break;
        case INFO:    os << "INFO"; break;
        case WARNING: os << "WARNING"; break;
        case ERROR:   os << "ERROR"; break;
        default:      os << "UNKNOWN"; break;
    }
    return os;
}

class Logger {
public:
    LOG_LEVEL log_level;
    const char* format;
public:
    Logger(LOG_LEVEL log_level, const char* format) : log_level(log_level), format(format) {}

    void log(LOG_LEVEL level, const char* msg) {
        if (level < this->log_level) return;

        this->print_prefix();
        std::cout << msg << "\n";
    }

    /* printf format support */
    template<typename... Args>
    void log(LOG_LEVEL level, const char* msg, Args&&... args) {
        if (level < this->log_level) return;

        char buffer[1024];
        std::snprintf(buffer, sizeof(buffer), msg, std::forward<Args>(args)...);

        this->print_prefix();
        std::cout << buffer << "\n";
    }
private:
    void print_prefix() {
        time_t timestamp = time(NULL);
        struct tm datetime = *localtime(&timestamp);
        char time[50];
        std::strftime(time, 50, format, &datetime);
        std::cout << "["<< this->log_level << " - " << time << "] ";
    }
};

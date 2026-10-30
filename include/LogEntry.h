#pragma once

#include "LogSeverity.h"

#include <cstddef>
#include <string>

struct LogTimestamp {
    int year{};
    int month{};
    int day{};
    int hour{};
    int minute{};
    int second{};

    friend bool operator==(const LogTimestamp& left, const LogTimestamp& right);
    friend bool operator<(const LogTimestamp& left, const LogTimestamp& right);
};

struct LogEntry {
    LogTimestamp timestamp;
    LogSeverity severity;
    std::string component;
    std::string message;
    std::size_t source_line{};
};

#pragma once

#include <string_view>

enum class LogSeverity {
    Info,
    Warn,
    Error,
};

bool try_parse_severity(std::string_view text, LogSeverity& severity);

#pragma once

#include "LogEntry.h"

#include <string>
#include <string_view>
#include <variant>

struct ParseDiagnostic {
    std::string reason;
};

using ParseResult = std::variant<LogEntry, ParseDiagnostic>;

class LogParser {
public:
    ParseResult parse(std::string_view line, std::size_t source_line = 0) const;
};

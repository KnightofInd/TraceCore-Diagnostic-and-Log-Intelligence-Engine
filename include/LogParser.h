#pragma once

#include "LogEntry.h"

#include <string>
#include <string_view>
#include <variant>

enum class InputFormat {
    Pipe,
    JsonLines,
    Csv,
};

struct ParseDiagnostic {
    std::string reason;
};

using ParseResult = std::variant<LogEntry, ParseDiagnostic>;

class LogParser {
public:
    ParseResult parse(std::string_view line, InputFormat format = InputFormat::Pipe,
                      std::size_t source_line = 0) const;
    ParseResult parse(std::string_view line, std::size_t source_line) const {
        return parse(line, InputFormat::Pipe, source_line);
    }
};

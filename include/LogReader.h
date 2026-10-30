#pragma once

#include "LogEntry.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

struct ReaderDiagnostic {
    std::size_t source_line{};
    std::string reason;
};

struct ReadResult {
    std::vector<LogEntry> entries;
    std::vector<ReaderDiagnostic> diagnostics;
    std::optional<std::string> fatal_error;
    bool strict_validation_failure{false};

    bool succeeded() const {
        return !fatal_error.has_value() && !strict_validation_failure;
    }
};

class LogReader {
public:
    ReadResult read(const std::string& path, bool strict) const;
};

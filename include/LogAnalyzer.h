#pragma once

#include "LogEntry.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

struct AnalysisFilter {
    std::optional<LogSeverity> severity;
    std::optional<std::string> component;
};

struct ComponentErrorCount {
    std::string component;
    std::size_t count{};
};

struct RepeatedError {
    std::string message;
    std::size_t count{};
};

struct AnalysisResult {
    std::size_t total_records{};
    std::size_t info_count{};
    std::size_t warn_count{};
    std::size_t error_count{};
    std::vector<ComponentErrorCount> errors_by_component;
    std::vector<RepeatedError> repeated_errors;
    std::optional<LogTimestamp> earliest_timestamp;
    std::optional<LogTimestamp> latest_timestamp;
};

class LogAnalyzer {
public:
    AnalysisResult analyze(const std::vector<LogEntry>& entries,
                           const AnalysisFilter& filter = {}) const;
};

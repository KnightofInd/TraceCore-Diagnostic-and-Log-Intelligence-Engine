#include "LogAnalyzer.h"

#include <map>

namespace {

bool matches(const LogEntry& entry, const AnalysisFilter& filter) {
    return (!filter.severity.has_value() || entry.severity == *filter.severity) &&
           (!filter.component.has_value() || entry.component == *filter.component);
}

}  // namespace

AnalysisResult LogAnalyzer::analyze(const std::vector<LogEntry>& entries,
                                    const AnalysisFilter& filter) const {
    AnalysisResult result;
    std::map<std::string, std::size_t> component_counts;
    std::map<std::string, std::size_t> error_messages;

    for (const auto& entry : entries) {
        if (!matches(entry, filter)) {
            continue;
        }

        ++result.total_records;
        switch (entry.severity) {
        case LogSeverity::Info:
            ++result.info_count;
            break;
        case LogSeverity::Warn:
            ++result.warn_count;
            break;
        case LogSeverity::Error:
            ++result.error_count;
            ++component_counts[entry.component];
            ++error_messages[entry.message];
            break;
        }

        if (!result.earliest_timestamp.has_value() ||
            entry.timestamp < *result.earliest_timestamp) {
            result.earliest_timestamp = entry.timestamp;
        }
        if (!result.latest_timestamp.has_value() ||
            *result.latest_timestamp < entry.timestamp) {
            result.latest_timestamp = entry.timestamp;
        }
    }

    for (const auto& [component, count] : component_counts) {
        result.errors_by_component.push_back({component, count});
    }
    for (const auto& [message, count] : error_messages) {
        if (count > 1) {
            result.repeated_errors.push_back({message, count});
        }
    }

    return result;
}

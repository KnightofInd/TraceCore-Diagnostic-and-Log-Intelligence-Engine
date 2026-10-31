#include "ReportFormatter.h"

#include <iomanip>
#include <sstream>

namespace {

std::string format_timestamp(const LogTimestamp& timestamp) {
    std::ostringstream output;
    output << std::setfill('0') << std::setw(4) << timestamp.year << '-'
           << std::setw(2) << timestamp.month << '-' << std::setw(2)
           << timestamp.day << ' ' << std::setw(2) << timestamp.hour << ':'
           << std::setw(2) << timestamp.minute << ':' << std::setw(2)
           << timestamp.second;
    return output.str();
}

std::string json_escape(const std::string& value) {
    std::ostringstream escaped;
    for (const unsigned char character : value) {
        switch (character) {
        case '"':
            escaped << "\\\"";
            break;
        case '\\':
            escaped << "\\\\";
            break;
        case '\b':
            escaped << "\\b";
            break;
        case '\f':
            escaped << "\\f";
            break;
        case '\n':
            escaped << "\\n";
            break;
        case '\r':
            escaped << "\\r";
            break;
        case '\t':
            escaped << "\\t";
            break;
        default:
            if (character < 0x20) {
                escaped << "\\u00" << std::hex << std::setw(2)
                        << std::setfill('0') << static_cast<int>(character)
                        << std::dec;
            } else {
                escaped << character;
            }
        }
    }
    return escaped.str();
}

}  // namespace

std::string ReportFormatter::format(
    const AnalysisResult& analysis,
    const std::vector<ReaderDiagnostic>& diagnostics) const {
    std::ostringstream output;
    output << "Log Analysis Report\n"
           << "===================\n"
           << "Total records: " << analysis.total_records << '\n'
           << "INFO: " << analysis.info_count << '\n'
           << "WARN: " << analysis.warn_count << '\n'
           << "ERROR: " << analysis.error_count << '\n';

    output << "\nErrors by component:\n";
    if (analysis.errors_by_component.empty()) {
        output << "  (none)\n";
    } else {
        for (const auto& component : analysis.errors_by_component) {
            output << "  " << component.component << ": " << component.count
                   << '\n';
        }
    }

    output << "\nRepeated errors:\n";
    if (analysis.repeated_errors.empty()) {
        output << "  (none)\n";
    } else {
        for (const auto& error : analysis.repeated_errors) {
            output << "  " << error.message << ": " << error.count << '\n';
        }
    }

    output << "\nTime range:\n";
    output << "  Earliest: "
           << (analysis.earliest_timestamp.has_value()
                   ? format_timestamp(*analysis.earliest_timestamp)
                   : "(none)")
           << '\n';
    output << "  Latest: "
           << (analysis.latest_timestamp.has_value()
                   ? format_timestamp(*analysis.latest_timestamp)
                   : "(none)")
           << '\n';

    output << "\nParse diagnostics: " << diagnostics.size() << '\n';
    for (const auto& diagnostic : diagnostics) {
        output << "  " << (diagnostic.source.empty() ? "" : diagnostic.source + ": ")
               << "Line " << diagnostic.source_line << ": "
               << diagnostic.reason << '\n';
    }
    return output.str();
}

std::string ReportFormatter::format_json(
    const AnalysisResult& analysis,
    const std::vector<ReaderDiagnostic>& diagnostics) const {
    std::ostringstream output;
    output << "{\n"
           << "  \"total_records\": " << analysis.total_records << ",\n"
           << "  \"severity_counts\": {\n"
           << "    \"INFO\": " << analysis.info_count << ",\n"
           << "    \"WARN\": " << analysis.warn_count << ",\n"
           << "    \"ERROR\": " << analysis.error_count << "\n"
           << "  },\n"
           << "  \"errors_by_component\": [";
    for (std::size_t index = 0; index < analysis.errors_by_component.size(); ++index) {
        const auto& item = analysis.errors_by_component[index];
        output << (index == 0 ? "\n" : ",\n")
               << "    {\"component\": \"" << json_escape(item.component)
               << "\", \"count\": " << item.count << "}";
    }
    if (!analysis.errors_by_component.empty()) {
        output << '\n';
    }
    output << "  ],\n"
           << "  \"repeated_errors\": [";
    for (std::size_t index = 0; index < analysis.repeated_errors.size(); ++index) {
        const auto& item = analysis.repeated_errors[index];
        output << (index == 0 ? "\n" : ",\n")
               << "    {\"message\": \"" << json_escape(item.message)
               << "\", \"count\": " << item.count << "}";
    }
    if (!analysis.repeated_errors.empty()) {
        output << '\n';
    }
    output << "  ],\n"
           << "  \"earliest_timestamp\": ";
    if (analysis.earliest_timestamp.has_value()) {
        output << '"' << format_timestamp(*analysis.earliest_timestamp) << '"';
    } else {
        output << "null";
    }
    output << ",\n  \"latest_timestamp\": ";
    if (analysis.latest_timestamp.has_value()) {
        output << '"' << format_timestamp(*analysis.latest_timestamp) << '"';
    } else {
        output << "null";
    }
    output << ",\n  \"parse_diagnostics\": [";
    for (std::size_t index = 0; index < diagnostics.size(); ++index) {
        const auto& item = diagnostics[index];
        output << (index == 0 ? "\n" : ",\n")
               << "    {\"source\": \"" << json_escape(item.source)
               << "\", \"line\": " << item.source_line
               << ", \"reason\": \"" << json_escape(item.reason) << "\"}";
    }
    if (!diagnostics.empty()) {
        output << '\n';
    }
    output << "  ]\n}\n";
    return output.str();
}

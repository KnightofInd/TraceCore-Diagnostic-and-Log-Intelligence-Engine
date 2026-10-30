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
        output << "  Line " << diagnostic.source_line << ": "
               << diagnostic.reason << '\n';
    }

    return output.str();
}

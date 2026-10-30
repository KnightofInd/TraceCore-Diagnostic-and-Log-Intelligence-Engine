#include "LogReader.h"

#include "LogParser.h"

#include <fstream>
#include <utility>

ReadResult LogReader::read(const std::string& path, bool strict) const {
    ReadResult result;
    std::ifstream input(path);
    if (!input.is_open()) {
        result.fatal_error = "unable to open input file: " + path;
        return result;
    }

    LogParser parser;
    std::string line;
    std::size_t source_line = 0;
    while (std::getline(input, line)) {
        ++source_line;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        ParseResult parsed = parser.parse(line, source_line);
        if (const auto* entry = std::get_if<LogEntry>(&parsed)) {
            result.entries.push_back(*entry);
            continue;
        }

        const auto& diagnostic = std::get<ParseDiagnostic>(parsed);
        result.diagnostics.push_back({source_line, diagnostic.reason});
        if (strict) {
            result.strict_validation_failure = true;
            break;
        }
    }

    if (input.bad()) {
        result.fatal_error = "failed while reading input file: " + path;
    }

    return result;
}

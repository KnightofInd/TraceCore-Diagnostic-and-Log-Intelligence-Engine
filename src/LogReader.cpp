#include "LogReader.h"

#include "LogParser.h"

#include <fstream>

ReadResult LogReader::read(const std::string& path, bool strict,
                           InputFormat format) const {
    std::ifstream input(path);
    if (!input.is_open()) {
        return ReadResult{
            {},
            {},
            "unable to open input file: " + path,
            false,
        };
    }

    ReadResult result = read(input, strict, format);
    for (auto& diagnostic : result.diagnostics) {
        diagnostic.source = path;
    }
    if (input.bad()) {
        result.fatal_error = "failed while reading input file: " + path;
    }
    return result;
}

ReadResult LogReader::read(std::istream& input, bool strict,
                           InputFormat format) const {
    ReadResult result;
    LogParser parser;
    std::string line;
    std::size_t source_line = 0;
    while (std::getline(input, line)) {
        ++source_line;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        ParseResult parsed = parser.parse(line, format, source_line);
        if (const auto* entry = std::get_if<LogEntry>(&parsed)) {
            result.entries.push_back(*entry);
            continue;
        }

        const auto& diagnostic = std::get<ParseDiagnostic>(parsed);
        result.diagnostics.push_back({source_line, diagnostic.reason, {}});
        if (strict) {
            result.strict_validation_failure = true;
            break;
        }
    }

    if (input.bad()) {
        result.fatal_error = "failed while reading input stream";
    }
    return result;
}

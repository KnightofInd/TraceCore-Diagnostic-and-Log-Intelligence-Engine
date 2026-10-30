#include "CommandLine.h"

#include "LogSeverity.h"

#include <string_view>
#include <iostream>
#include <utility>
#include <variant>

namespace {

CommandLineResult error(std::string message) {
    return CommandLineError{std::move(message)};
}

bool has_value(const std::string& value) {
    return !value.empty();
}

}  // namespace

CommandLineResult parse_command_line(int argc, char* argv[]) {
    CommandLineOptions options;
    bool input_seen = false;
    bool output_seen = false;
    bool severity_seen = false;
    bool component_seen = false;
    bool strict_seen = false;
    bool help_seen = false;

    for (int index = 1; index < argc; ++index) {
        const std::string_view argument{argv[index]};
        if (argument == "--help") {
            if (help_seen) {
                return error("--help may only be specified once");
            }
            help_seen = true;
            options.help = true;
            continue;
        }
        if (argument == "--strict") {
            if (strict_seen) {
                return error("--strict may only be specified once");
            }
            strict_seen = true;
            options.strict = true;
            continue;
        }

        auto value_for = [&]() -> std::optional<std::string> {
            if (index + 1 >= argc || !has_value(argv[index + 1])) {
                return std::nullopt;
            }
            ++index;
            return std::string(argv[index]);
        };

        if (argument == "--input") {
            if (input_seen) {
                return error("--input may only be specified once");
            }
            auto value = value_for();
            if (!value.has_value()) {
                return error("--input requires a non-empty path");
            }
            options.input_path = *value;
            input_seen = true;
        } else if (argument == "--output") {
            if (output_seen) {
                return error("--output may only be specified once");
            }
            auto value = value_for();
            if (!value.has_value()) {
                return error("--output requires a non-empty path");
            }
            options.output_path = *value;
            output_seen = true;
        } else if (argument == "--severity") {
            if (severity_seen) {
                return error("--severity may only be specified once");
            }
            auto value = value_for();
            if (!value.has_value()) {
                return error("--severity requires INFO, WARN, or ERROR");
            }
            LogSeverity severity;
            if (!try_parse_severity(*value, severity)) {
                return error("invalid severity: " + *value);
            }
            options.filter.severity = severity;
            severity_seen = true;
        } else if (argument == "--component") {
            if (component_seen) {
                return error("--component may only be specified once");
            }
            auto value = value_for();
            if (!value.has_value()) {
                return error("--component requires a non-empty name");
            }
            options.filter.component = *value;
            component_seen = true;
        } else {
            return error("unknown option: " + std::string(argument));
        }
    }

    if (options.help) {
        return options;
    }
    if (!input_seen) {
        return error("--input is required");
    }
    return options;
}

void print_help() {
    std::cout << "Usage: log_analyzer --input PATH [options]\n"
                 "\n"
                 "Options:\n"
                 "  --input PATH          Input log file (required)\n"
                 "  --severity LEVEL      Filter by INFO, WARN, or ERROR\n"
                 "  --component NAME      Filter by component\n"
                 "  --strict              Fail on the first malformed record\n"
                 "  --output PATH         Write the report to a file\n"
                 "  --help                Show this help message\n";
}

#pragma once

#include "LogAnalyzer.h"

#include <optional>
#include <string>
#include <variant>

struct CommandLineOptions {
    bool help{false};
    bool strict{false};
    std::string input_path;
    std::optional<std::string> output_path;
    AnalysisFilter filter;
};

struct CommandLineError {
    std::string message;
};

using CommandLineResult = std::variant<CommandLineOptions, CommandLineError>;

CommandLineResult parse_command_line(int argc, char* argv[]);
void print_help();

#pragma once

#include "LogAnalyzer.h"
#include "LogParser.h"

#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class OutputFormat {
    Text,
    Json,
};

struct CommandLineOptions {
    bool help{false};
    bool strict{false};
    std::vector<std::string> input_paths;
    std::optional<std::string> output_path;
    AnalysisFilter filter;
    OutputFormat format{OutputFormat::Text};
    InputFormat input_format{InputFormat::Pipe};
};

struct CommandLineError {
    std::string message;
};

using CommandLineResult = std::variant<CommandLineOptions, CommandLineError>;

CommandLineResult parse_command_line(int argc, char* argv[]);
void print_help();

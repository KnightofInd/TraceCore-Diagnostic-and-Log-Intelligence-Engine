#include "CommandLine.h"

#include "LogReader.h"
#include "ReportFormatter.h"

#include <fstream>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace {

constexpr int kSuccess = 0;
constexpr int kProcessingFailure = 1;
constexpr int kUsageError = 2;
constexpr int kStrictValidationFailure = 3;

int write_report(const std::string& report,
                 const std::optional<std::string>& output_path) {
    if (!output_path.has_value()) {
        std::cout << report;
        return kSuccess;
    }

    std::ofstream output(*output_path);
    if (!output.is_open()) {
        std::cerr << "Unable to open output file: " << *output_path << '\n';
        return kProcessingFailure;
    }
    output << report;
    if (!output.good()) {
        std::cerr << "Failed while writing output file: " << *output_path << '\n';
        return kProcessingFailure;
    }
    return kSuccess;
}

}  // namespace

int main(int argc, char* argv[]) {
    const CommandLineResult command_line = parse_command_line(argc, argv);
    if (const auto* command_error = std::get_if<CommandLineError>(&command_line)) {
        std::cerr << "Error: " << command_error->message << '\n'
                  << "Use --help for usage.\n";
        return kUsageError;
    }

    const auto& options = std::get<CommandLineOptions>(command_line);
    if (options.help) {
        print_help();
        return kSuccess;
    }

    LogReader reader;
    ReadResult read_result;
    for (const auto& input_path : options.input_paths) {
        ReadResult current = input_path == "-"
                                 ? reader.read(std::cin, options.strict,
                                               options.input_format)
                                 : reader.read(input_path, options.strict,
                                               options.input_format);
        if (input_path == "-") {
            for (auto& diagnostic : current.diagnostics) {
                diagnostic.source = "<stdin>";
            }
        }
        read_result.entries.insert(read_result.entries.end(),
                                   current.entries.begin(), current.entries.end());
        read_result.diagnostics.insert(read_result.diagnostics.end(),
                                       current.diagnostics.begin(),
                                       current.diagnostics.end());
        if (current.fatal_error.has_value()) {
            read_result.fatal_error = current.fatal_error;
            break;
        }
        if (current.strict_validation_failure) {
            read_result.strict_validation_failure = true;
            break;
        }
    }
    if (read_result.fatal_error.has_value()) {
        std::cerr << "Error: " << *read_result.fatal_error << '\n';
        return kProcessingFailure;
    }
    if (read_result.strict_validation_failure) {
        for (const auto& diagnostic : read_result.diagnostics) {
            std::cerr << (diagnostic.source.empty() ? "" : diagnostic.source + ": ")
                      << "Line " << diagnostic.source_line << ": "
                      << diagnostic.reason << '\n';
        }
        return kStrictValidationFailure;
    }

    const AnalysisResult analysis =
        LogAnalyzer{}.analyze(read_result.entries, options.filter);
    const ReportFormatter formatter;
    const std::string report = options.format == OutputFormat::Json
                                    ? formatter.format_json(
                                          analysis, read_result.diagnostics)
                                    : formatter.format(
                                          analysis, read_result.diagnostics);
    return write_report(report, options.output_path);
}

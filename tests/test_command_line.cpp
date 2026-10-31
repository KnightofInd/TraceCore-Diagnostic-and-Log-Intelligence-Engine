#include "CommandLine.h"

#include <gtest/gtest.h>

#include <iterator>
#include <variant>

TEST(CommandLineTest, ParsesAllSupportedOptions) {
    const char* arguments[] = {
        "log_analyzer", "--input", "input.log", "--severity", "ERROR",
        "--component", "AUDIO", "--strict", "--output", "report.txt"};

    const auto result = parse_command_line(
        static_cast<int>(std::size(arguments)), const_cast<char**>(arguments));

    ASSERT_TRUE(std::holds_alternative<CommandLineOptions>(result));
    const auto& options = std::get<CommandLineOptions>(result);
    ASSERT_EQ(options.input_paths.size(), 1);
    EXPECT_EQ(options.input_paths[0], "input.log");
    EXPECT_EQ(options.output_path, "report.txt");
    EXPECT_EQ(options.filter.severity, LogSeverity::Error);
    EXPECT_EQ(options.filter.component, "AUDIO");
    EXPECT_TRUE(options.strict);
}

TEST(CommandLineTest, RequiresInputUnlessHelpIsRequested) {
    const char* arguments[] = {"log_analyzer"};
    const auto result =
        parse_command_line(1, const_cast<char**>(arguments));
    ASSERT_TRUE(std::holds_alternative<CommandLineError>(result));
    EXPECT_NE(std::get<CommandLineError>(result).message.find("--input"),
              std::string::npos);
}

TEST(CommandLineTest, AcceptsMultipleInputsAndJsonFormat) {
    const char* arguments[] = {
        "log_analyzer", "--input", "a.log", "--input", "b.log",
        "--format", "json"};
    const auto result =
        parse_command_line(7, const_cast<char**>(arguments));
    ASSERT_TRUE(std::holds_alternative<CommandLineOptions>(result));
    const auto& options = std::get<CommandLineOptions>(result);
    ASSERT_EQ(options.input_paths.size(), 2);
    EXPECT_EQ(options.format, OutputFormat::Json);
}

TEST(CommandLineTest, AcceptsInputFormat) {
    const char* arguments[] = {
        "log_analyzer", "--input", "events.jsonl", "--input-format", "jsonl"};
    const auto result =
        parse_command_line(5, const_cast<char**>(arguments));
    ASSERT_TRUE(std::holds_alternative<CommandLineOptions>(result));
    EXPECT_EQ(std::get<CommandLineOptions>(result).input_format,
              InputFormat::JsonLines);
}

TEST(CommandLineTest, RejectsUnknownInvalidAndConflictingOptions) {
    const char* unknown[] = {"log_analyzer", "--input", "a.log", "--unknown"};
    EXPECT_TRUE(std::holds_alternative<CommandLineError>(
        parse_command_line(4, const_cast<char**>(unknown))));

    const char* invalid_format[] = {
        "log_analyzer", "--input", "a.log", "--format", "xml"};
    EXPECT_TRUE(std::holds_alternative<CommandLineError>(
        parse_command_line(5, const_cast<char**>(invalid_format))));

    const char* mixed_input[] = {
        "log_analyzer", "--input", "-", "--input", "b.log"};
    EXPECT_TRUE(std::holds_alternative<CommandLineError>(
        parse_command_line(5, const_cast<char**>(mixed_input))));
}

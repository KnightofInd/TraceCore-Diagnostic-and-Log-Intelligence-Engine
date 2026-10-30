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
    EXPECT_EQ(options.input_path, "input.log");
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

TEST(CommandLineTest, RejectsUnknownAndRepeatedOptions) {
    const char* unknown[] = {"log_analyzer", "--input", "a.log", "--unknown"};
    EXPECT_TRUE(std::holds_alternative<CommandLineError>(
        parse_command_line(4, const_cast<char**>(unknown))));

    const char* repeated[] = {
        "log_analyzer", "--input", "a.log", "--input", "b.log"};
    EXPECT_TRUE(std::holds_alternative<CommandLineError>(
        parse_command_line(5, const_cast<char**>(repeated))));
}

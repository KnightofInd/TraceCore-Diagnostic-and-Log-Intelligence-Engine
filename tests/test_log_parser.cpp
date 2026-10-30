#include "LogParser.h"

#include <gtest/gtest.h>

#include <string>
#include <variant>

namespace {

const LogEntry& entry_from(ParseResult& result) {
    return std::get<LogEntry>(result);
}

}  // namespace

TEST(LogParserTest, ParsesValidRecordAndPreservesSourceLine) {
    LogParser parser;
    auto result = parser.parse(
        "2026-10-30 09:15:01|ERROR|AUDIO|Device failed|retrying", 7);

    ASSERT_TRUE(std::holds_alternative<LogEntry>(result));
    const auto& entry = entry_from(result);
    EXPECT_EQ(entry.timestamp.year, 2026);
    EXPECT_EQ(entry.timestamp.month, 10);
    EXPECT_EQ(entry.timestamp.day, 30);
    EXPECT_EQ(entry.severity, LogSeverity::Error);
    EXPECT_EQ(entry.component, "AUDIO");
    EXPECT_EQ(entry.message, "Device failed|retrying");
    EXPECT_EQ(entry.source_line, 7);
}

TEST(LogParserTest, ParsesAllSupportedSeverities) {
    LogParser parser;
    for (const std::string severity : {"INFO", "WARN", "ERROR"}) {
        const auto result = parser.parse(
            "2026-10-30 09:15:01|" + severity + "|SYSTEM|Ready");
        EXPECT_TRUE(std::holds_alternative<LogEntry>(result));
    }
}

TEST(LogParserTest, RejectsInvalidDateAndTime) {
    LogParser parser;
    EXPECT_TRUE(std::holds_alternative<ParseDiagnostic>(
        parser.parse("2026-02-29 09:15:01|INFO|SYSTEM|Ready")));
    EXPECT_TRUE(std::holds_alternative<ParseDiagnostic>(
        parser.parse("2026-10-30 24:00:00|INFO|SYSTEM|Ready")));
}

TEST(LogParserTest, RejectsMissingFieldsAndEmptyValues) {
    LogParser parser;
    for (const std::string line : {
             "2026-10-30 09:15:01|INFO|SYSTEM",
             "2026-10-30 09:15:01|INFO||Ready",
             "2026-10-30 09:15:01|INFO|SYSTEM|"}) {
        EXPECT_TRUE(std::holds_alternative<ParseDiagnostic>(parser.parse(line)));
    }
}

TEST(LogParserTest, RejectsUnknownSeverity) {
    LogParser parser;
    const auto result =
        parser.parse("2026-10-30 09:15:01|DEBUG|SYSTEM|Ready");
    ASSERT_TRUE(std::holds_alternative<ParseDiagnostic>(result));
    EXPECT_NE(std::get<ParseDiagnostic>(result).reason.find("unknown severity"),
              std::string::npos);
}

TEST(LogParserTest, RejectsBlankRecord) {
    LogParser parser;
    EXPECT_TRUE(std::holds_alternative<ParseDiagnostic>(parser.parse("")));
}

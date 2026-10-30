#include "LogAnalyzer.h"

#include <gtest/gtest.h>

#include <vector>

namespace {

LogEntry entry(int day, LogSeverity severity, const char* component,
               const char* message) {
    return {{2026, 10, day, 9, 15, 0}, severity, component, message, 0};
}

}  // namespace

TEST(LogAnalyzerTest, CalculatesCountsAndTimeRange) {
    const std::vector<LogEntry> entries{
        entry(30, LogSeverity::Error, "ZETA", "Timeout"),
        entry(28, LogSeverity::Info, "SYSTEM", "Started"),
        entry(29, LogSeverity::Error, "AUDIO", "Device failed"),
        entry(30, LogSeverity::Error, "AUDIO", "Device failed"),
        entry(30, LogSeverity::Warn, "NETWORK", "Slow"),
    };

    const auto result = LogAnalyzer{}.analyze(entries);

    EXPECT_EQ(result.total_records, 5);
    EXPECT_EQ(result.info_count, 1);
    EXPECT_EQ(result.warn_count, 1);
    EXPECT_EQ(result.error_count, 3);
    ASSERT_EQ(result.errors_by_component.size(), 2);
    EXPECT_EQ(result.errors_by_component[0].component, "AUDIO");
    EXPECT_EQ(result.errors_by_component[1].component, "ZETA");
    ASSERT_EQ(result.repeated_errors.size(), 1);
    EXPECT_EQ(result.repeated_errors[0].message, "Device failed");
    EXPECT_EQ(result.repeated_errors[0].count, 2);
    ASSERT_TRUE(result.earliest_timestamp.has_value());
    ASSERT_TRUE(result.latest_timestamp.has_value());
    EXPECT_EQ(result.earliest_timestamp->day, 28);
    EXPECT_EQ(result.latest_timestamp->day, 30);
}

TEST(LogAnalyzerTest, AppliesCombinedFilters) {
    const std::vector<LogEntry> entries{
        entry(30, LogSeverity::Error, "AUDIO", "Failed"),
        entry(30, LogSeverity::Error, "NETWORK", "Failed"),
        entry(30, LogSeverity::Info, "AUDIO", "Ready"),
    };

    AnalysisFilter filter{LogSeverity::Error, "AUDIO"};
    const auto result = LogAnalyzer{}.analyze(entries, filter);

    EXPECT_EQ(result.total_records, 1);
    EXPECT_EQ(result.error_count, 1);
    EXPECT_EQ(result.errors_by_component.size(), 1);
    EXPECT_EQ(result.errors_by_component[0].component, "AUDIO");
}

TEST(LogAnalyzerTest, ReturnsEmptyResultForNoMatchingRecords) {
    const std::vector<LogEntry> entries{
        entry(30, LogSeverity::Info, "SYSTEM", "Ready")};

    const auto result =
        LogAnalyzer{}.analyze(entries, AnalysisFilter{LogSeverity::Error, {}});

    EXPECT_EQ(result.total_records, 0);
    EXPECT_FALSE(result.earliest_timestamp.has_value());
    EXPECT_FALSE(result.latest_timestamp.has_value());
    EXPECT_TRUE(result.errors_by_component.empty());
}

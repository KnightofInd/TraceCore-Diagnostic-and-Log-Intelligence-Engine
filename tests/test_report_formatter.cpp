#include "ReportFormatter.h"

#include <gtest/gtest.h>

TEST(ReportFormatterTest, FormatsEmptyAnalysis) {
    const std::string report = ReportFormatter{}.format(AnalysisResult{});

    EXPECT_NE(report.find("Total records: 0"), std::string::npos);
    EXPECT_NE(report.find("Errors by component:\n  (none)"), std::string::npos);
    EXPECT_NE(report.find("Earliest: (none)"), std::string::npos);
    EXPECT_NE(report.find("Latest: (none)"), std::string::npos);
    EXPECT_NE(report.find("Parse diagnostics: 0"), std::string::npos);
}

TEST(ReportFormatterTest, FormatsStatisticsAndDiagnostics) {
    AnalysisResult analysis;
    analysis.total_records = 2;
    analysis.info_count = 1;
    analysis.error_count = 1;
    analysis.errors_by_component.push_back({"AUDIO", 1});
    analysis.earliest_timestamp = LogTimestamp{2026, 10, 30, 9, 15, 1};
    analysis.latest_timestamp = LogTimestamp{2026, 10, 30, 9, 15, 7};

    const std::vector<ReaderDiagnostic> diagnostics{
        {4, "invalid timestamp", "app.log"}};
    const std::string report =
        ReportFormatter{}.format(analysis, diagnostics);

    EXPECT_NE(report.find("INFO: 1"), std::string::npos);
    EXPECT_NE(report.find("AUDIO: 1"), std::string::npos);
    EXPECT_NE(report.find("2026-10-30 09:15:01"), std::string::npos);
    EXPECT_NE(report.find("app.log: Line 4: invalid timestamp"), std::string::npos);
}

TEST(ReportFormatterTest, FormatsJsonAndEscapesStrings) {
    AnalysisResult analysis;
    analysis.total_records = 1;
    analysis.error_count = 1;
    analysis.errors_by_component.push_back({"AUDIO", 1});
    analysis.repeated_errors.push_back({"Device \"failed\"", 2});
    const std::vector<ReaderDiagnostic> diagnostics{
        {3, "bad\\line", "app.log"}};

    const std::string report =
        ReportFormatter{}.format_json(analysis, diagnostics);

    EXPECT_NE(report.find("\"total_records\": 1"), std::string::npos);
    EXPECT_NE(report.find("Device \\\"failed\\\""), std::string::npos);
    EXPECT_NE(report.find("bad\\\\line"), std::string::npos);
}

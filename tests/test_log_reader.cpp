#include "LogReader.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TemporaryLogFile {
public:
    explicit TemporaryLogFile(const std::string& contents)
        : path_(std::filesystem::temp_directory_path() /
                ("cpp_log_analyzer_" +
                 std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) +
                 ".log")) {
        std::ofstream output(path_);
        EXPECT_TRUE(output.is_open());
        output << contents;
        EXPECT_TRUE(output.good());
    }

    ~TemporaryLogFile() {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }

    const std::string path() const {
        return path_.string();
    }

private:
    std::filesystem::path path_;
};

}  // namespace

TEST(LogReaderTest, ReadsValidRecordsAndTracksLineNumbers) {
    TemporaryLogFile file(
        "2026-10-30 09:15:01|INFO|SYSTEM|Started\n"
        "2026-10-30 09:15:02|ERROR|AUDIO|Failed\n");

    const ReadResult result = LogReader{}.read(file.path(), false);

    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.entries.size(), 2);
    EXPECT_EQ(result.entries[0].source_line, 1);
    EXPECT_EQ(result.entries[1].source_line, 2);
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST(LogReaderTest, LenientModeSkipsMalformedRecordsAndContinues) {
    TemporaryLogFile file(
        "2026-10-30 09:15:01|INFO|SYSTEM|Started\n"
        "not a valid record\n"
        "2026-10-30 09:15:03|WARN|NETWORK|Slow\n");

    const ReadResult result = LogReader{}.read(file.path(), false);

    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.entries.size(), 2);
    ASSERT_EQ(result.diagnostics.size(), 1);
    EXPECT_EQ(result.diagnostics[0].source_line, 2);
}

TEST(LogReaderTest, StrictModeStopsAtFirstMalformedRecord) {
    TemporaryLogFile file(
        "2026-10-30 09:15:01|INFO|SYSTEM|Started\n"
        "not a valid record\n"
        "2026-10-30 09:15:03|WARN|NETWORK|Slow\n");

    const ReadResult result = LogReader{}.read(file.path(), true);

    EXPECT_FALSE(result.succeeded());
    EXPECT_TRUE(result.strict_validation_failure);
    ASSERT_EQ(result.entries.size(), 1);
    ASSERT_EQ(result.diagnostics.size(), 1);
    EXPECT_EQ(result.diagnostics[0].source_line, 2);
}

TEST(LogReaderTest, EmptyFileIsSuccessful) {
    TemporaryLogFile file("");

    const ReadResult result = LogReader{}.read(file.path(), false);

    EXPECT_TRUE(result.succeeded());
    EXPECT_TRUE(result.entries.empty());
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST(LogReaderTest, MissingFileIsFatal) {
    const ReadResult result =
        LogReader{}.read("path-that-does-not-exist.log", false);

    EXPECT_FALSE(result.succeeded());
    ASSERT_TRUE(result.fatal_error.has_value());
    EXPECT_NE(result.fatal_error->find("unable to open"), std::string::npos);
}

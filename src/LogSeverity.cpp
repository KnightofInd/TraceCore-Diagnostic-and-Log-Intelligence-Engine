#include "LogSeverity.h"

bool try_parse_severity(std::string_view text, LogSeverity& severity) {
    if (text == "INFO") {
        severity = LogSeverity::Info;
        return true;
    }
    if (text == "WARN") {
        severity = LogSeverity::Warn;
        return true;
    }
    if (text == "ERROR") {
        severity = LogSeverity::Error;
        return true;
    }
    return false;
}

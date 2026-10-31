#include "LogParser.h"

#include <array>
#include <charconv>
#include <cctype>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

std::string trim(std::string_view text) {
    const auto first = text.find_first_not_of(" \t");
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = text.find_last_not_of(" \t");
    return std::string(text.substr(first, last - first + 1));
}

bool is_digits(std::string_view text, std::size_t count) {
    if (text.size() != count) {
        return false;
    }
    for (const char character : text) {
        if (!std::isdigit(static_cast<unsigned char>(character))) {
            return false;
        }
    }
    return true;
}

bool parse_integer(std::string_view text, int& value) {
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

bool is_leap_year(int year) {
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

int days_in_month(int year, int month) {
    static constexpr std::array<int, 12> days{
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days[static_cast<std::size_t>(month - 1)];
}

bool parse_timestamp(std::string_view text, LogTimestamp& timestamp) {
    if (text.size() != 19 || text[4] != '-' || text[7] != '-' ||
        text[10] != ' ' || text[13] != ':' || text[16] != ':' ||
        !is_digits(text.substr(0, 4), 4) ||
        !is_digits(text.substr(5, 2), 2) ||
        !is_digits(text.substr(8, 2), 2) ||
        !is_digits(text.substr(11, 2), 2) ||
        !is_digits(text.substr(14, 2), 2) ||
        !is_digits(text.substr(17, 2), 2)) {
        return false;
    }

    return parse_integer(text.substr(0, 4), timestamp.year) &&
           parse_integer(text.substr(5, 2), timestamp.month) &&
           parse_integer(text.substr(8, 2), timestamp.day) &&
           parse_integer(text.substr(11, 2), timestamp.hour) &&
           parse_integer(text.substr(14, 2), timestamp.minute) &&
           parse_integer(text.substr(17, 2), timestamp.second) &&
           timestamp.year >= 1 &&
           timestamp.month >= 1 && timestamp.month <= 12 &&
           timestamp.day >= 1 && timestamp.day <= days_in_month(timestamp.year, timestamp.month) &&
           timestamp.hour >= 0 && timestamp.hour <= 23 &&
           timestamp.minute >= 0 && timestamp.minute <= 59 &&
           timestamp.second >= 0 && timestamp.second <= 59;
}

ParseResult failure(std::string reason) {
    return ParseDiagnostic{std::move(reason)};
}

bool parse_json_string(std::string_view text, std::size_t& index,
                       std::string& value) {
    if (index >= text.size() || text[index] != '"') {
        return false;
    }
    ++index;
    while (index < text.size() && text[index] != '"') {
        if (text[index] == '\\') {
            ++index;
            if (index >= text.size()) {
                return false;
            }
            switch (text[index]) {
            case '"': value += '"'; break;
            case '\\': value += '\\'; break;
            case '/': value += '/'; break;
            case 'b': value += '\b'; break;
            case 'f': value += '\f'; break;
            case 'n': value += '\n'; break;
            case 'r': value += '\r'; break;
            case 't': value += '\t'; break;
            default: return false;
            }
        } else {
            value += text[index];
        }
        ++index;
    }
    if (index >= text.size()) {
        return false;
    }
    ++index;
    return true;
}

bool json_field(std::string_view line, std::string_view key,
                std::string& value) {
    const std::string token = "\"" + std::string(key) + "\"";
    const auto key_position = line.find(token);
    if (key_position == std::string_view::npos) {
        return false;
    }
    auto index = line.find(':', key_position + token.size());
    if (index == std::string_view::npos) {
        return false;
    }
    ++index;
    while (index < line.size() && (line[index] == ' ' || line[index] == '\t')) {
        ++index;
    }
    return parse_json_string(line, index, value);
}

ParseResult parse_json_lines(std::string_view line, std::size_t source_line) {
    std::string timestamp;
    std::string severity;
    std::string component;
    std::string message;
    if (line.empty() || !json_field(line, "timestamp", timestamp) ||
        !json_field(line, "severity", severity) ||
        !json_field(line, "component", component) ||
        !json_field(line, "message", message)) {
        return failure("invalid JSON Lines record; expected string fields "
                       "timestamp, severity, component, and message");
    }
    return LogParser{}.parse(timestamp + "|" + severity + "|" + component +
                                 "|" + message,
                             InputFormat::Pipe, source_line);
}

ParseResult parse_csv(std::string_view line, std::size_t source_line) {
    std::vector<std::string> fields;
    std::string field;
    bool quoted = false;
    for (std::size_t index = 0; index <= line.size(); ++index) {
        const char character = index < line.size() ? line[index] : ',';
        if (character == '"' && (quoted || field.empty())) {
            if (quoted && index + 1 < line.size() && line[index + 1] == '"') {
                field += '"';
                ++index;
            } else {
                quoted = !quoted;
            }
        } else if (character == ',' && !quoted) {
            fields.push_back(trim(field));
            field.clear();
        } else {
            field += character;
        }
    }
    if (quoted || fields.size() != 4) {
        return failure("invalid CSV record; expected timestamp,severity,"
                       "component,message");
    }
    return LogParser{}.parse(fields[0] + "|" + fields[1] + "|" + fields[2] +
                                 "|" + fields[3],
                             InputFormat::Pipe, source_line);
}

}  // namespace

bool operator==(const LogTimestamp& left, const LogTimestamp& right) {
    return left.year == right.year && left.month == right.month &&
           left.day == right.day && left.hour == right.hour &&
           left.minute == right.minute && left.second == right.second;
}

bool operator<(const LogTimestamp& left, const LogTimestamp& right) {
    return std::tie(left.year, left.month, left.day, left.hour, left.minute, left.second) <
           std::tie(right.year, right.month, right.day, right.hour, right.minute, right.second);
}

ParseResult LogParser::parse(std::string_view line, InputFormat format,
                             std::size_t source_line) const {
    if (format == InputFormat::JsonLines) {
        return parse_json_lines(line, source_line);
    }
    if (format == InputFormat::Csv) {
        return parse_csv(line, source_line);
    }
    if (line.empty()) {
        return failure("record is empty");
    }

    const auto first_separator = line.find('|');
    const auto second_separator =
        first_separator == std::string_view::npos ? std::string_view::npos
                                                  : line.find('|', first_separator + 1);
    const auto third_separator =
        second_separator == std::string_view::npos ? std::string_view::npos
                                                   : line.find('|', second_separator + 1);

    if (first_separator == std::string_view::npos ||
        second_separator == std::string_view::npos ||
        third_separator == std::string_view::npos) {
        return failure("record must contain four pipe-separated fields");
    }

    LogTimestamp timestamp;
    if (!parse_timestamp(line.substr(0, first_separator), timestamp)) {
        return failure("invalid timestamp; expected YYYY-MM-DD HH:MM:SS");
    }

    LogSeverity severity;
    if (!try_parse_severity(
            line.substr(first_separator + 1, second_separator - first_separator - 1),
            severity)) {
        return failure("unknown severity; expected INFO, WARN, or ERROR");
    }

    const auto component =
        line.substr(second_separator + 1, third_separator - second_separator - 1);
    if (component.empty()) {
        return failure("component must not be empty");
    }

    const auto message = line.substr(third_separator + 1);
    if (message.empty()) {
        return failure("message must not be empty");
    }

    return LogEntry{
        timestamp,
        severity,
        std::string(component),
        std::string(message),
        source_line,
    };
}

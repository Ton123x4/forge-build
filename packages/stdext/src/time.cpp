#include "stdext/time.hpp"
#include <chrono>
#include <ctime>
#include <format>

namespace stdext {
    time::datetime::datetime(const std::tm& time) {
        this->year = time.tm_year + 1900;
        this->month = time.tm_mon + 1;
        this->day = time.tm_mday;
        this->hour = time.tm_hour;
        this->minute = time.tm_min;
        this->second = time.tm_sec;
    }

    auto time::datetime::to_string() const -> std::string {
        return std::format("{:02}/{:02}/{:04} {:02}:{:02}:{:02}", day, month, year, hour, minute, second);
    }

    time::stopwatch::stopwatch() {
        reset();
    }

    void time::stopwatch::reset() {
        start_time = std::chrono::steady_clock::now();
    }

    time::time_t time::stopwatch::elapsed_milliseconds() const {
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

        return duration.count();
    }

    time::time_t time::stopwatch::elapsed_seconds() const {
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(elapsed);

        return duration.count();
    }

    time::time_t time::unix_seconds() {
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

        return duration.count();
    }

    time::time_t time::unix_milliseconds() {
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());

        return duration.count();
    }

    time::time_t time::monotonic_seconds() {
        static auto application_start = std::chrono::steady_clock::now();

        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - application_start);

        return duration.count();
    }

    time::time_t time::monotonic_milliseconds() {
        static auto application_start = std::chrono::steady_clock::now();

        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - application_start);

        return duration.count();
    }

    time::datetime time::unix_to_local(time::time_t unix_seconds) {
        auto time = static_cast<std::time_t>(unix_seconds);
        auto local_time = std::tm();

#ifdef _WIN32
        localtime_s(&local_time, &time);
#else
        localtime_r(&time, &local_time);
#endif

        return datetime(local_time);
    }

    time::datetime time::unix_to_utc(time::time_t unix_seconds) {
        auto time = static_cast<std::time_t>(unix_seconds);
        auto utc_time = std::tm();

#ifdef _WIN32
        gmtime_s(&utc_time, &time);
#else
        gmtime_r(&time, &utc_time);
#endif

        return datetime(utc_time);
    }

    std::string time::unix_to_iso_seconds(time_t unix_seconds) {
        return std::format("{:%FT%TZ}", time::to_zoned_time(unix_seconds));
    }

    std::string time::unix_to_iso_milliseconds(time_t unix_seconds) {
        return std::format("{:%FT%T}Z", time::to_zoned_time<std::chrono::milliseconds>(unix_seconds));
    }

    time::datetime time::now_local() {
        return unix_to_local(unix_seconds());
    }

    time::datetime time::now_utc() {
        return unix_to_utc(unix_seconds());
    }

    std::string time::iso_seconds() {
        auto date_time = now_utc();
        auto currente_date = std::format("{:04}-{:02}-{:02}", date_time.year, date_time.month, date_time.day);
        auto currente_time = std::format("{:02}:{:02}:{:02}", date_time.hour, date_time.minute, date_time.second);

        return std::format("{}T{}Z", currente_date, currente_time);
    }

    std::string time::iso_milliseconds() {
        auto total_ms = unix_milliseconds();
        auto seconds = total_ms / 1000;
        auto milliseconds = total_ms % 1000;
        auto date_time = unix_to_utc(seconds);

        auto currente_date = std::format("{:04}-{:02}-{:02}", date_time.year, date_time.month, date_time.day);
        auto currente_time = std::format("{:02}:{:02}:{:02}.{:03}", date_time.hour, date_time.minute, date_time.second, milliseconds);

        return std::format("{}T{}Z", currente_date, currente_time);
    }
}

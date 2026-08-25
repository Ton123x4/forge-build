#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace stdext::time {
    using time_t = uint64_t;

    struct datetime {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;

        datetime(const std::tm& time);
        datetime() = default;

        std::string to_string() const;
    };

    class stopwatch {
       public:
        stopwatch();

        void reset();

        time_t elapsed_milliseconds() const;
        time_t elapsed_seconds() const;

       private:
        std::chrono::steady_clock::time_point start_time;
    };

    std::string iso_seconds();
    std::string iso_milliseconds();

    time_t unix_seconds();
    time_t unix_milliseconds();

    time_t monotonic_seconds();
    time_t monotonic_milliseconds();

    datetime now_local();
    datetime now_utc();

    datetime unix_to_local(time_t unix_seconds);
    datetime unix_to_utc(time_t unix_seconds);

    std::string unix_to_iso_seconds(time_t unix_seconds);
    std::string unix_to_iso_milliseconds(time_t unix_seconds);

    template <typename FloorType = std::chrono::seconds>
    inline auto unix_to_chrono(time_t unix_seconds) {
        auto time_point = std::chrono::system_clock::from_time_t(unix_seconds);
        auto duration = std::chrono::floor<FloorType>(time_point);

        return duration;
    }

    template <typename FloorType = std::chrono::seconds>
    inline auto to_zoned_time(time_t timestamp) {
        return std::chrono::zoned_time{ std::chrono::current_zone(), unix_to_chrono<FloorType>(timestamp) };
    }
}

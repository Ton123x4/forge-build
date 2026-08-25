#pragma once

#include <array>
#include <utility>
#include <functional>

namespace stdext {
    template<typename Func>
    struct defer_t {
        Func func;

        ~defer_t() {
            func();
        }
    };

    template<typename Func>
    defer_t<Func> defer(Func&& func) {
        return { std::forward<Func>(func) };
    }


    template<std::size_t N>
    constexpr std::array<char, N> bytes_array(const char (&array)[N]) {
        auto result = std::array<char, N>();

        for (std::size_t i = 0; i < N; ++i) {
            result[i] = array[i];
        }

        return result;
    }

    template<std::size_t N>
    constexpr std::array<char, N + 1> text_array(const char (&array)[N]) {
        auto result = std::array<char, N + 1>();

        for (std::size_t i = 0; i < N; ++i) {
            result[i] = array[i];
        }

        result[N] = '\0';

        return result;
    }


    template <typename T>
    auto pipe(T&& value) {
        return std::forward<T>(value);
    }

    template <typename T, typename F, typename... Fn>
    auto pipe(T&& value, F&& first_fn, Fn&&... remaining_fns) {
        return pipe(
            std::invoke(
                std::forward<F>(first_fn),
                std::forward<T>(value)
            ),
            std::forward<Fn>(remaining_fns)...
        );
    }

    template <typename T, typename F>
    T tap(T&& value, F&& func) {
        std::invoke(std::forward<F>(func), value);
        return std::forward<T>(value);
    }

    template <typename T, typename F>
    T transform_if(bool condition, T&& value, F&& func) {
        if (condition) {
            return std::invoke(std::forward<F>(func), std::forward<T>(value));
        }

        return std::forward<T>(value);
    }

    template<typename T>
    constexpr T value_or(T&& value, T&& fallback) {
        return value ? std::forward<T>(value) : std::forward<T>(fallback);
    }

    template <typename T, typename F>
    T value_or_else(bool condition, T&& value, F&& fallback) {
        if (condition) {
            return std::forward<T>(value);
        }

        return std::invoke(std::forward<F>(fallback));
    }
}

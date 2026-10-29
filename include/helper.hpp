#pragma once
#include <numeric>
#include <stdexcept>
#include<string>
#include<vector>
template <typename T, typename... Args>
constexpr bool matchesAny(const T &target, const Args &...items) {
    return ((target == items) || ...);
}
template <typename T, typename... Args>
bool matchesAll(const T &target, const Args &...items) {
    return ((target == items) && ...);
}
template <typename T>
    requires std::is_arithmetic_v<T>
long double average(const std::vector<T> &numbers) {
    if (numbers.empty()) {
        throw std::invalid_argument("Cannot average an empty vector");
    }

    const long double sum =
        std::accumulate(numbers.begin(), numbers.end(), 0.0L);

    return sum / numbers.size();
}
void exitError(std::string Error);

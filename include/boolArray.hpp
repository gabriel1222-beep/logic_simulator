#pragma once
#include <array>
#include <cstdint>
enum class Mode {
    compact, // packed into uint8_t bytes
    fast     // use integer words
};

template <std::size_t N>
using FastStorage = std::conditional_t<
    (N <= 8), std::uint8_t,
    std::conditional_t<
        (N <= 16), std::uint16_t,
        std::conditional_t<
            (N <= 32), std::uint32_t,
            std::conditional_t<
                (N <= 64), std::uint64_t,
                std::array<std::uint64_t, N / 64 + (N % 64 != 0)>>>>>;

template <std::size_t N, Mode mode = Mode::compact>
using BoolStorage = std::conditional_t<
    mode == Mode::compact,
    std::array<std::uint8_t, N / 8 + (N % 8 != 0)>,
    FastStorage<N>>;

template <std::size_t N, Mode mode = Mode::compact>
struct BoolArray {
    using storage_type = BoolStorage<N, mode>;

    storage_type storage{};
    template <typename... U>
    constexpr BoolArray(U... values) {
        static_assert(sizeof...(U) <= N, "Too many initial values");

        constexpr std::size_t count = sizeof...(U);
        const std::array<bool, count> initial{static_cast<bool>(values)...};

        for (std::size_t i = 0; i < count; ++i) {

            if constexpr (mode == Mode::compact) {
                auto &byte = storage[i / 8];
                const auto mask =
                    static_cast<std::uint8_t>(1u << (i % 8));

                if (initial[i])
                    byte |= mask;
                else
                    byte &= static_cast<std::uint8_t>(~mask);
            } else if constexpr (N <= 64) {
                const auto mask =
                    static_cast<storage_type>(storage_type{1} << i);

                if (initial[i])
                    storage = static_cast<storage_type>(storage | mask);
                else
                    storage = static_cast<storage_type>(storage & ~mask);
            } else {
                auto &word = storage[i / 64];
                const auto mask = std::uint64_t{1} << (i % 64);

                if (initial[i])
                    word |= mask;
                else
                    word &= ~mask;
            }
        }
    }

    void setValue(std::size_t index, bool value = true) {
        if (index >= N) return;

        if constexpr (mode == Mode::compact) {
            auto &byte = storage[index / 8];
            const auto mask =
                static_cast<std::uint8_t>(1u << (index % 8));

            if (value)
                byte |= mask;
            else
                byte &= static_cast<std::uint8_t>(~mask);
        } else if constexpr (N <= 64) {
            const auto mask =
                static_cast<storage_type>(storage_type{1} << index);

            if (value)
                storage = static_cast<storage_type>(storage | mask);
            else
                storage = static_cast<storage_type>(storage & ~mask);
        } else {
            auto &word = storage[index / 64];
            const auto mask = std::uint64_t{1} << (index % 64);

            if (value)
                word |= mask;
            else
                word &= ~mask;
        }
    }
    template <unsigned index>
    void setValue(bool value = true) {
        if (index >= N) return;

        if constexpr (mode == Mode::compact) {
            auto &byte = storage[index / 8];
            constexpr auto mask =
                static_cast<std::uint8_t>(1u << (index % 8));

            if (value)
                byte |= mask;
            else
                byte &= static_cast<std::uint8_t>(~mask);
        } else if constexpr (N <= 64) {
            constexpr auto mask =
                static_cast<storage_type>(storage_type{1} << index);

            if (value)
                storage = static_cast<storage_type>(storage | mask);
            else
                storage = static_cast<storage_type>(storage & ~mask);
        } else {
            auto &word = storage[index / 64];
            constexpr auto mask = std::uint64_t{1} << (index % 64);

            if (value)
                word |= mask;
            else
                word &= ~mask;
        }
    }

    bool at(std::size_t index) const {
        if (index >= N) return false;

        if constexpr (mode == Mode::compact) {
            return (storage[index / 8] & (1u << (index % 8))) != 0;
        } else if constexpr (N <= 64) {
            return (storage & (storage_type{1} << index)) != 0;
        } else {
            return (storage[index / 64] &
                    (std::uint64_t{1} << (index % 64))) != 0;
        }
    }
    bool operator[](std::size_t i) const {
        if constexpr (mode == Mode::compact) {
            return (storage[i / 8] & (1u << (i % 8))) != 0;
        } else if constexpr (N <= 64) {
            return (storage & (storage_type{1} << i)) != 0;
        } else {
            return (storage[i / 64] &
                    (std::uint64_t{1} << (i % 64))) != 0;
        }
    }
    consteval std::size_t getSize() {
        return N;
    }
    consteval std::size_t getSpaceUsage() {
        return sizeof(storage);
    }
};

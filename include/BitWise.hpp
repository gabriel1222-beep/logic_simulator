#pragma once
#include <concepts>
#include <limits>
#include <cstddef>
namespace BitWise {
template <std::size_t Bit, std::unsigned_integral T>
constexpr T bit_mask() {
    static_assert(!std::same_as<T, bool>,
                  "T must be an unsigned integer type, not bool");
    static_assert(Bit < std::numeric_limits<T>::digits,
                  "Bit position is outside the type");

    return static_cast<T>(T{1} << Bit);
}

template <std::size_t Bit, std::unsigned_integral T>
constexpr bool read_bit(T byte) {
    return (byte & bit_mask<Bit, T>()) != 0;
}

template <std::size_t Bit, std::unsigned_integral T>
constexpr void turn_on(T &byte) {
    byte = static_cast<T>(byte | bit_mask<Bit, T>());
}

template <std::size_t Bit, std::unsigned_integral T>
constexpr void turn_off(T &byte) {
    byte = static_cast<T>(byte & static_cast<T>(~bit_mask<Bit, T>()));
}

template <std::size_t Bit, std::unsigned_integral T>
constexpr void flip_bit(T &byte) {
    byte = static_cast<T>(byte ^ bit_mask<Bit, T>());
}

template <std::size_t Bit, std::unsigned_integral T>
constexpr void set_bit(T &byte, bool value) {
    if (value)
        turn_on<Bit>(byte);
    else
        turn_off<Bit>(byte);
}
template <std::size_t Pos, std::size_t Width, std::unsigned_integral T>
constexpr T setField(T word, T value) {
    constexpr std::size_t digits = std::numeric_limits<T>::digits;

    static_assert(Width > 0);
    static_assert(Pos + Width <= digits);

    constexpr T low_mask = [] {
        if constexpr (Width == digits)
            return std::numeric_limits<T>::max();
        else
            return (T{1} << Width) - 1;
    }();

    constexpr T mask = low_mask << Pos;

    return (word & ~mask) | ((value & low_mask) << Pos);
}
template <std::size_t Pos, std::size_t Width, std::unsigned_integral T>
constexpr T getField(T word) {
    constexpr std::size_t digits = std::numeric_limits<T>::digits;

    static_assert(Width > 0);
    static_assert(Pos + Width <= digits);

    constexpr T low_mask = [] {
        if constexpr (Width == digits)
            return std::numeric_limits<T>::max();
        else
            return (T{1} << Width) - 1;
    }();

    return (word >> Pos) & low_mask;
}
} // namespace BitWise

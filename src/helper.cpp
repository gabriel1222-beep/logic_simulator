#include <cstdint>
consteval bool bitOn(const std::uint8_t &value, const std::uint8_t bit) {
  static_assert(bit < 8,"bitOn() Error: Bit must be < 8");
    return ((value & (std::uint8_t{1} << bit)) != 0);
}
 
 

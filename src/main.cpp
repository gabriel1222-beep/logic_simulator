#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <sys/types.h>
#include <type_traits>
#include <variant>
#include <vector>

#define BIT_ON(value, bit) (((value) & (std::uint8_t{1} << bit)) != 0)
template <class X, class... Values>
consteval bool orChain(const X &x, const Values &...values) {
    return ((x == values) || ...);
}
template <const std::uint8_t bit>
constexpr bool bitOn(const std::uint8_t &value) {
    static_assert(bit < 8, "Bit must be less than 8");
    return ((value & (std::uint8_t{1} << bit)) != 0);
}

constexpr void setBit(std::uint8_t &value, const std::uint8_t bit) {
    if (bit < 8) {
        value ^= std::uint8_t{1} << bit;
    }
}

constexpr void clearBit(std::uint8_t &value, const std::uint8_t bit) {
    if (bit < 8) {
        value &= static_cast<std::uint8_t>(~(std::uint8_t{1} << bit));
    }
}
template <auto typeA, auto typeB>
constexpr bool EqualType() {
    return std::is_same_v<decltype(typeA), decltype(typeB)>;
}
#define LOGIC_GATE_COMPONENTS(X) \
    X(And)                       \
    X(Or)                        \
    X(Not)\
    X(Xor)
#define OTHER_COMPONENTS(X) \
    X(OnInput)
enum class Component_type_ids : std::uint8_t {
#define X(component) component,
    LOGIC_GATE_COMPONENTS(X)
#undef X
#define X(component) component,
        OTHER_COMPONENTS(X)
#undef X
};
namespace Components {
namespace LogicGates {
#define X(component) inline constexpr Component_type_ids component = Component_type_ids::component;
LOGIC_GATE_COMPONENTS(X)
#undef X
} // namespace LogicGates
template <Component_type_ids id>
consteval bool islogicComponent() {
#define X(component) \
    if constexpr (id == Component_type_ids::component) return 1;
    LOGIC_GATE_COMPONENTS(X)
#undef X
    return 0;
}

namespace other {
#define X(component) inline constexpr Component_type_ids component = Component_type_ids::component;
OTHER_COMPONENTS(X)
#undef X
} // namespace other
template <Component_type_ids id>
constexpr bool component_exists() {
#define X(component)                                     \
    if constexpr (id == Component_type_ids::component) { \
        return 1;                                        \
    }
    LOGIC_GATE_COMPONENTS(X)

#undef X
#define X(component)                                     \
    if constexpr (id == Component_type_ids::component) { \
        return 1;                                        \
    }
    OTHER_COMPONENTS(X)

#undef X
    return 0;
}
} // namespace Components

template <std::uint8_t Entries, Component_type_ids type>
struct LogicGate {
    static_assert(Components::islogicComponent<type>(), "Invalid type for LogicComponent");
    static_assert(type != Component_type_ids::And || Entries >= 2, "And gate must have at least 2 entries");
    static_assert(type != Component_type_ids::Or || Entries >= 2, "Or gate must have at least 2 entries");
    static_assert(type != Component_type_ids::Xor || Entries >= 2, "Xor gate must have at least 2 entries");
    static_assert(type != Component_type_ids::Not || Entries == 1, "Not gate must have exactly 1 entry");

    std::uint8_t entries[Entries]{};
    constexpr bool out() {
        if constexpr (type == Component_type_ids::And) {
            return entries[0] && entries[1];
        } else if constexpr (type == Component_type_ids::Or) {
            return entries[0] || entries[1];
        } else if constexpr (type == Component_type_ids::Not) {
            return !entries[0];
        } else if constexpr (type == Component_type_ids::Xor) {
            return (entries[0] ^ !entries[1]);
        }
    }
};
struct Circuit {
  private:
    unsigned long int ids = 0;

  public:
};

int main() {
    LogicGate<2, Components::LogicGates::And> gate;
    return 0;
}

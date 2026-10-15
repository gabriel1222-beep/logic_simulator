#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
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
#define COMPONENTS(X) \
    X(And)            \
    X(Or)             \
    X(Not)            \
    X(Xor)            \
    X(ON_IN)          \
    X(Button)
enum class Components_enum : std::uint8_t {
#define X(component) component,
    COMPONENTS(X)
#undef X
        UNDEFINED_COMPONENT
};
template <Components_enum T>
concept LogicGateComponent =
    T == Components_enum::And ||
    T == Components_enum::Or ||
    T == Components_enum::Not ||
    T == Components_enum::Xor ||
    T == Components_enum::Button ||
    T == Components_enum::ON_IN;

template <Components_enum type>
    requires LogicGateComponent<type>
struct LogicGate {
    static constexpr std::uint8_t input_number = type == Components_enum::ON_IN || type == Components_enum::Button ? 0 : type == Components_enum::Not ? 1
                                                                                                                                                      : 2;
    bool *entries[input_number]{};
    bool inputs[input_number]{};
    bool output = 0;
    bool readInput(std::size_t i) const {
        return entries[i] ? *entries[i] : 0;
    }

    constexpr bool *getOutput() {
        if constexpr (input_number == 0) {
            if constexpr (type == Components_enum::ON_IN) {
                output = 1;
                return &output;
            } else if constexpr (type == Components_enum::Button) {
                output = !output;
                return &output;
            }
        } else if constexpr (input_number == 1) {
            const bool A = readInput(0);
            if constexpr (type == Components_enum::Not) {
                output = !A;
            }

        } else if (input_number == 2) {
            const bool A = readInput(0);
            const bool B = readInput(1);
            if constexpr (type == Components_enum::And) {
                output = A && B;
            } else if constexpr (type == Components_enum::Or) {
                output = A || B;
            } else if constexpr (type == Components_enum::Xor) {
                output = A ^ B;
            }
        }
        return &output;
    }
};
struct Circuit {
    using LogicComponent = std::variant<LogicGate<Components_enum ::And>,
                                        LogicGate<Components_enum ::Or>,
                                        LogicGate<Components_enum ::Not>,
                                        LogicGate<Components_enum ::Xor>,
                                        LogicGate<Components_enum ::ON_IN>>;
    using Component = std::variant<LogicComponent>;
    std::vector<Component> Components;
    bool *Component_getOutput(LogicComponent &comp) {
        return std::visit(
            [](auto &gate) { return gate.getOutput(); },
            comp);
    }
    void addComponent(LogicComponent comp) {
        Components.push_back(comp);
    }
    void connect(LogicComponent &dest, bool dest_entry, LogicComponent &src) {
        bool *source_output = std::visit(
            [](auto &gate) { return gate.getOutput(); },
            src);
        std::visit(
            [dest_entry, source_output](auto &gate) {
                gate.entries[dest_entry] = source_output;
            },
            dest);
    }
    void step(uint32_t steps) {
        for (uint32_t i = 0; i < steps; i++) {

            for (Component &comp : Components) {
                const bool output = std::visit(
                    [](auto &logic_component) {
                        return std::visit(
                            [](auto &gate) {
                                return *gate.getOutput();
                            },
                            logic_component);
                    },
                    comp);
            }
        }
    }
};
int main() {
    Circuit::LogicComponent AndGateA{LogicGate<Components_enum::And>{}};
    Circuit::LogicComponent AndGateB{LogicGate<Components_enum::And>{}};
    Circuit::LogicComponent OnIN{LogicGate<Components_enum::ON_IN>{}};
    Circuit circuit;
    circuit.addComponent(AndGateA);
    circuit.addComponent(AndGateB);
    circuit.addComponent(OnIN);
    circuit.connect(AndGateA, 0, AndGateB);
    circuit.connect(AndGateA, 1, AndGateB);
    circuit.connect(AndGateB, 1, OnIN);
    circuit.connect(AndGateB, 0, OnIN);

    bool *GateAOut = std::visit(
        [](auto &gate) { return gate.getOutput(); },
        AndGateA);
    circuit.step(1000);
    std::cout << GateAOut << std::endl;
    std::cout << *GateAOut;

    return 0;
}

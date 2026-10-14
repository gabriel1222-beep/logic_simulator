#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
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
    X(Not)                       \
    X(Xor)                       \
    X(ON_IN)
enum class LogicGates : std::uint8_t {
#define X(component) component,
    LOGIC_GATE_COMPONENTS(X)
#undef X
};
template <LogicGates type>
struct LogicGate {
    bool *entries[2]{};
    bool inputs[2]{};
    bool output = 0;
      bool readInput(std::size_t i) const {
        return entries[i] ? *entries[i] : 0;
        }

    constexpr bool *getOutput() {
        if constexpr (type == LogicGates::ON_IN) {
            output = 1;
            return &output;
        }
        const bool A = readInput(0);
        const bool B = readInput(1);
        if constexpr (type == LogicGates::And) {
            output = A && B;
        } else if constexpr (type == LogicGates::Or) {
            output = A || B;
        } else if constexpr (type == LogicGates::Not) {
            output = !A;
        } else if constexpr (type == LogicGates::Xor) {
            output = A ^ B;
        }
        return &output;
    }
};
struct Circuit {
    using Component = std::variant<LogicGate<LogicGates::And>, LogicGate<LogicGates::Or>, LogicGate<LogicGates::Not>, LogicGate<LogicGates::Xor>, LogicGate<LogicGates::ON_IN>>;
    using Component_ptrs = Component *;
    std::vector<Component_ptrs> Components;
    bool *Component_getOutput(Component &comp) {
        return std::visit(
            [](auto &gate) { return gate.getOutput(); },
            comp);
    }
    void addComponent(Component *comp) {
        Components.push_back(comp);
    }
    void connect(Component &dest, bool dest_entry, Component &src) {
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

            for (Component *component_ptr : Components) {
                if (component_ptr) {
                    std::visit([](auto &gate) {
                        gate.getOutput();
                    },
                               *component_ptr);
                }
            }
        }
    }
};
int main() {
    Circuit::Component AndGateA{LogicGate<LogicGates::And>{}};
    Circuit::Component AndGateB{LogicGate<LogicGates::And>{}};
    Circuit::Component OnIN{LogicGate<LogicGates::ON_IN>{}};
    Circuit circuit;
    circuit.addComponent(&AndGateA);
    circuit.addComponent(&AndGateB);
    circuit.addComponent(&OnIN);
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

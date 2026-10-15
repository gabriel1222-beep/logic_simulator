#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <ostream>
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
    static constexpr std::uint8_t input_number = (type == Components_enum::ON_IN || type == Components_enum::Button) ? 0 : type == Components_enum::Not ? 1
                                                                                                                                                        : 2;
    bool entries[input_number]{};
    bool output = 0;
    constexpr void evaluate() {
        if constexpr (input_number == 0) {
            if constexpr (type == Components_enum::ON_IN) {
                output = 1;
            } else if constexpr (type == Components_enum::Button) {
                output = !output;
            }
        } else if constexpr (input_number == 1) {
            const bool A = entries[0];
            if constexpr (type == Components_enum::Not) {
                output = !A;
            }

        } else if (input_number == 2) {
            const bool A = entries[0];
            const bool B = entries[1];
            if constexpr (type == Components_enum::And) {
                output = A && B;
            } else if constexpr (type == Components_enum::Or) {
                output = A || B;
            } else if constexpr (type == Components_enum::Xor) {
                output = A ^ B;
            }
        }
    }
    bool getOutput() {
        return output;
    }
};
struct Circuit {
    using LogicComponent = std::variant<LogicGate<Components_enum ::And>,
                                        LogicGate<Components_enum ::Or>,
                                        LogicGate<Components_enum ::Not>,
                                        LogicGate<Components_enum ::Xor>,
                                        LogicGate<Components_enum ::ON_IN>,
                                        LogicGate<Components_enum::Button>>;
    std::vector<LogicComponent> Components;
    using id = std::size_t;
    struct Connector {
        id dest_entry_id;
        id dest_id;
        id src_id;
    };
    std::vector<Connector> connections;
    bool Component_getOutput(id id) {
        return std::visit(
            [](auto &gate) { return gate.getOutput(); },
            Components[id]);
    }
    void Component_Evaluate(LogicComponent &comp) {
        std::visit(
            [](auto &gate) { gate.evaluate(); },
            comp);
    }
    id addComponent(LogicComponent comp) {
        Components.push_back(comp);
        return Components.size() - 1;
    }
    void connect(id dest, std::uint8_t dest_entry, id src) {
        Connector connector;
        connector.dest_id = dest;
        connector.src_id = src;
        connector.dest_entry_id = dest_entry;
        connections.push_back(connector);
    }
    //TODO: Simulate the circuit
    void step(uint32_t steps) {
    }
};
int main() {
    Circuit::LogicComponent AndGateA{LogicGate<Components_enum::And>{}};
    Circuit::LogicComponent AndGateB{LogicGate<Components_enum::And>{}};
    Circuit::LogicComponent OnIN{LogicGate<Components_enum::ON_IN>{}};
    Circuit::LogicComponent notG{LogicGate<Components_enum::Not>{}};
    Circuit circuit;
    auto andAId = circuit.addComponent(AndGateA);
    auto andBId = circuit.addComponent(AndGateB);
    auto onInId = circuit.addComponent(OnIN);
    auto notGId = circuit.addComponent(notG);
    circuit.connect(andBId, 1, onInId);
    circuit.connect(andBId, 0, onInId);
    circuit.connect(andAId, 0, andBId);
    circuit.connect(andAId, 1, andBId);
    circuit.connect(notGId, 0, andAId);

    bool GateAOut = circuit.Component_getOutput(andAId);
    bool GateBOut = circuit.Component_getOutput(andBId);
    bool NotGateOut = circuit.Component_getOutput(notGId);
    circuit.step(1000);
    std::cout << GateAOut << std::endl;
    std::cout << GateBOut << std::endl;
    std::cout << NotGateOut << std::endl;

    return 0;
}

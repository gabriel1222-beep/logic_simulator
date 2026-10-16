#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sys/types.h>
#include <variant>
#include <vector>

#define BIT_ON(value, bit) (((value) & (std::uint8_t{1} << bit)) != 0)
template <class F>
void repeat_fwrd(std::size_t size, const F &&f) {
    for (std::size_t i = 0; i < size; ++i)
        f(i);
}
template <class F>
void repeat_bckrd(std::size_t size, const F &&f) {
    for (std::size_t i = size; i > 0; i--)
        f(i);
}

enum class Components_enum : std::uint8_t {
    And,
    Or,
    Not,
    Xor,
    ON_IN,
    Button,
};
bool SameComponent(Components_enum component_typeA, Components_enum component_typeB) {
    return (component_typeA == component_typeB) ? true : false;
}
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
    static constexpr std::uint8_t number_of_entries = (type == Components_enum::ON_IN || type == Components_enum::Button) ? 0 : type == Components_enum::Not ? 1
                                                                                                                                                             : 2;
    bool entries[number_of_entries]{};
    bool output = 0;
    constexpr void evaluate() {
        if constexpr (number_of_entries == 0) {
            if constexpr (type == Components_enum::ON_IN) {
                output = 1;
            } else if constexpr (type == Components_enum::Button) {
                output = !output;
            }
        } else if constexpr (number_of_entries == 1) {
            const bool A = entries[0];
            if constexpr (type == Components_enum::Not) {
                output = !A;
            }
        } else if constexpr (number_of_entries == 2) {
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
        std::uint8_t dest_entry_id;
        id dest_id,src_id;
        Connector(id dest, std::uint8_t dest_entry, id src) : dest_id(dest), dest_entry_id(dest_entry), src_id(src) {}
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
    template <Components_enum type>
        requires LogicGateComponent<type>
    id addComponent() {
        Components.emplace_back(std::in_place_type<LogicGate<type>>);
        return Components.size()-1;
    }
    void connect(id dest_id, std::uint8_t dest_entry, id src_id) {
        connections.emplace_back(dest_id, dest_entry, src_id);
    }
    void step(std::size_t steps) {
        repeat_fwrd(steps, [&](std::size_t) {
            for (std::size_t i = 0; i < Components.size(); i++) {
                Component_Evaluate(Components[i]);
                const bool output = Component_getOutput(i);
                for (const auto &c : connections) {
                    if (c.src_id != i) continue;
                    auto &dest = Components.at(c.dest_id);
                    std::visit([c, output](auto &gate) {
                        if (c.dest_entry_id >= gate.number_of_entries) throw std::out_of_range("Invalid entry id");
                        gate.entries[c.dest_entry_id] = output;
                    },dest);
                }
            }
        });
    }
};
int main() {
    Circuit circuit;
    auto andAId = circuit.addComponent<Components_enum::And>();
    auto andBId = circuit.addComponent<Components_enum::And>();
    auto onInId = circuit.addComponent<Components_enum::ON_IN>();
    auto notGId = circuit.addComponent<Components_enum::Not>();
    circuit.connect(andBId, 1, onInId);
    circuit.connect(andBId, 0, onInId);
    circuit.connect(andAId, 0, andBId);
    circuit.connect(andAId, 1, andBId);
    circuit.connect(notGId, 0, andAId);

    circuit.step(1000);
    bool GateAOut = circuit.Component_getOutput(andAId);
    bool GateBOut = circuit.Component_getOutput(andBId);
    bool NotGateOut = circuit.Component_getOutput(notGId);
    std::cout << GateAOut << std::endl;
    std::cout << GateBOut << std::endl;
    std::cout << NotGateOut << std::endl;

    return 0;
}

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/format.h>
#include <iostream>
#include <ostream>
#include <sys/types.h>
#include <variant>
#include <vector>
#define READ_BIT(byte, bit) ((((byte) >> (bit)) & 1u) != 0u)
void turn_on(std::uint8_t &byte, unsigned int bit) {
    byte |= 1u << bit;
}

void turn_off(std::uint8_t &byte, unsigned int bit) {
    byte &= ~(1u << bit);
}

void flip_bit(std::uint8_t &byte, unsigned int bit) {
    byte ^= 1u << bit;
}

void set_bit(std::uint8_t &byte, unsigned int bit, bool value) {
    if (value)
        turn_on(byte, bit);
    else
        turn_off(byte, bit);
}

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

namespace Enums {
enum class Components_enum : std::uint8_t {
    And,
    Or,
    Not,
    Xor,
    ON_IN,
    Button,
};
}
bool SameComponent(Enums::Components_enum component_typeA, Enums::Components_enum component_typeB) {
    return (component_typeA == component_typeB) ? true : false;
}
#define LOGIC_GATE_OUTPUT_BIT 2
#define LOGIC_GATE_OUTPUT_ON 0b00000100
#define LOGIC_GATE_A_BIT 0
#define LOGIC_GATE_B_BIT 1
#define LOGIC_GATE_MAX_OUTGOING_CONNECTIONS 14
struct Gate {
    uint8_t in = 0b00000000;
    Gate() : in(0b00000000) {}
    Gate(uint8_t value) : in(value) {}
    std::uint8_t getConnectionCounter() {
        return (in >> 3) & 0x1F;
    }
    void setCounter(std::uint8_t value) {
        in = static_cast<std::uint8_t>(
            (in & 0b00000111) | ((value & 0b00000111) << 3));
    }
    void increaseConnectionCounter() {
        std::uint8_t n = getConnectionCounter();
        n++;
        setCounter(n);
    }
};
struct NotGate : Gate {
    std::uint8_t numberOfEntries() {
        return 1;
    }
    void evaluate() {
        flip_bit(in, LOGIC_GATE_OUTPUT_BIT);
    }
    bool getOutput() {
        return READ_BIT(in, LOGIC_GATE_OUTPUT_BIT);
    }
};
struct AndGate : Gate {

    std::uint8_t numberOfEntries() {
        return 2;
    }
    void evaluate() {
        if (READ_BIT(in, LOGIC_GATE_A_BIT) && READ_BIT(in, LOGIC_GATE_B_BIT)) {
            turn_on(in, LOGIC_GATE_OUTPUT_BIT);
        } else {
            turn_off(in, LOGIC_GATE_OUTPUT_BIT);
        }
    }
    bool getOutput() {
        return READ_BIT(in, LOGIC_GATE_OUTPUT_BIT);
    }
};
struct OrGate : Gate {

    std::uint8_t numberOfEntries() {
        return 2;
    }
    void evaluate() {
        if (READ_BIT(in, LOGIC_GATE_A_BIT) || READ_BIT(in, LOGIC_GATE_B_BIT)) {
            turn_on(in, LOGIC_GATE_OUTPUT_BIT);
        } else {
            turn_off(in, LOGIC_GATE_OUTPUT_BIT);
        }
    }
    bool getOutput() {
        return READ_BIT(in, LOGIC_GATE_OUTPUT_BIT);
    }
};
struct XorGate : Gate {

    std::uint8_t numberOfEntries() {
        return 2;
    }
    void evaluate() {
        if (READ_BIT(in, LOGIC_GATE_A_BIT) ^ READ_BIT(in, LOGIC_GATE_B_BIT)) {
            turn_on(in, LOGIC_GATE_OUTPUT_BIT);
        } else {
            turn_off(in, LOGIC_GATE_OUTPUT_BIT);
        }
    }
    bool getOutput() {
        return READ_BIT(in, LOGIC_GATE_OUTPUT_BIT);
    }
};
struct Button : Gate {

    std::uint8_t numberOfEntries() {
        return 0;
    }
    void evaluate() {
        flip_bit(in, LOGIC_GATE_OUTPUT_BIT);
    }
    bool getOutput() {
        return READ_BIT(in, LOGIC_GATE_OUTPUT_BIT);
    }
};
struct OnIn : Gate {
    std::uint8_t numberOfEntries() {
        return 0;
    }
    OnIn() : Gate(LOGIC_GATE_OUTPUT_ON) {}
    void evaluate() {}
    bool getOutput() {
        return READ_BIT(in, LOGIC_GATE_OUTPUT_BIT);
    }
};
struct In : Gate {
    std::uint8_t numberOfEntries() {
        return 1;
    }
    void evaluate() {}
    bool getOutput() {
        return READ_BIT(in, LOGIC_GATE_OUTPUT_BIT);
    }
};
struct Out : Gate {
    std::uint8_t numberOfEntries() {
        return 1;
    }
    void evaluate() {}
    bool getOutput() {
        return READ_BIT(in, LOGIC_GATE_OUTPUT_BIT);
    }
};
struct Circuit {
    using LogicComponent = std::variant<AndGate,
                                        OrGate,
                                        NotGate,
                                        XorGate,
                                        OnIn,
                                        Button, In, Out>;
    using id = std::uint64_t;
    struct alignas(64) Component {
        id dest_ids[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
        std::uint8_t dest_entry[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
        LogicComponent component;
        Component(LogicComponent component) : component(component) {}
        void connect(id dest_id, std::uint8_t dest_entry) {
            std::visit([&](auto &gate) {
                uint8_t n = gate.getConnectionCounter();
                this->dest_ids[n] = dest_id;
                this->dest_entry[n] = dest_entry;
                n++;
                gate.setCounter(n);
            },
                       component);
        }
        std::uint8_t outgoingConnections(){
            
            return std::visit([&](auto &gate) {
                return gate.getConnectionCounter();
            },
                       component);
        }
        void evaluate() {
            std::visit(
                [](auto &gate) { gate.evaluate(); },
                component);
        }
        bool getOutput() {
            return std::visit(
                [](auto &gate) { return gate.getOutput(); },
                component);
        }
        std::size_t printSize() {
            std::cout << fmt::format("LogicComponent size = {}\n", sizeof(Component::component));
            std::cout << fmt::format("Dest_entry size = {}\n", sizeof(Component::dest_entry));
            std::cout << fmt::format("Dest_ids size = {}\n", sizeof(Component::dest_ids));
            std::cout << fmt::format("Estimated Component size = {}\n", sizeof(Component::dest_ids) + sizeof(Component::dest_entry) + sizeof(Component::component));
            return sizeof(Component::dest_ids) + sizeof(Component::dest_entry) + sizeof(Component::component);
        }
    };
    void CacheLinesNeeded(long double bytes) {
        std::cout << fmt::format("{} bytes needs {} cache lines\n", bytes, bytes / 64);
    }
    struct Data {
        std::vector<Component> Components;
        template <class type>
        id addComponent() {
            Components.emplace_back(LogicComponent(std::in_place_type<type>));
            return Components.size() - 1;
        }
        void connect(id dest_id, std::uint8_t dest_entry, id src_id) {
            Components.at(src_id).connect(dest_id, dest_entry);
        }
        void setOutput(id dest_id, std::uint8_t dest_entry_id, const bool out) {
            auto &dest = Components[dest_id];
            std::visit([dest_entry_id, dest_id, out](auto &gate) {
                if (dest_entry_id >= gate.numberOfEntries()) throw std::out_of_range("Invalid entry id");
                set_bit(gate.in, dest_entry_id, out);
            },
                       dest.component);
        }
        Component getComponent(id id) {
            return Components[id];
        }
        std::size_t getinputNumber() {
            auto ins = 0;
            for (auto &component : Components) {
                if (std::holds_alternative<In>(component.component)) {
                    ins++;
                }
            }
            return ins;
        }
        std::size_t getoutputNumber() {
            auto outs = 0;
            for (auto &component : Components) {
                if (std::holds_alternative<Out>(component.component)) {
                    outs++;
                }
            }
            return outs;
        }
    };
    Data data;
    void startSimulation() {
        bool running = 1;
        while (running) {
            for (auto &component : data.Components) {
                component.evaluate();
                const bool output = component.getOutput();
                for (std::size_t i = 0; i < component.outgoingConnections(); i++) {
                    data.setOutput(component.dest_ids[i], component.dest_entry[i], output);
                }
            }
        }
    }
};
struct Simulator {
    std::vector<Circuit::Component> Components;
    std::vector<Circuit> Circuits;
    std::size_t circuits = 0;
    void ExtractComponentsfromCircuit(Circuit &circuit) {
        Components.insert(Components.end(), circuit.data.Components.begin(), circuit.data.Components.end());
    }
    std::size_t addCircuit(Circuit &circuit) {
        Circuits.push_back(circuit);
        return Circuits.size() - 1;
    }
    void connect(std::size_t circuit_dest_id, std::uint8_t circuit_dest_entry, std::size_t circuit_src_id, std::uint8_t circuit_src_output) {
    }
};
Circuit XorGate() {
    Circuit circuit;
    auto andA = circuit.data.addComponent<AndGate>();
    auto andB = circuit.data.addComponent<AndGate>();
    auto orA = circuit.data.addComponent<OrGate>();
    auto notA = circuit.data.addComponent<NotGate>();
    auto notB = circuit.data.addComponent<NotGate>();
    auto inA = circuit.data.addComponent<In>();
    auto inB = circuit.data.addComponent<In>();
    auto out = circuit.data.addComponent<In>();
    circuit.data.connect(notA, 0, inA);
    circuit.data.connect(andA, 0, notA);
    circuit.data.connect(andA, 1, inB); // AndA = notA and B
    circuit.data.connect(notB, 0, inB);
    circuit.data.connect(andB, 0, notB);
    circuit.data.connect(andB, 1, inA); // AndB = notB and A
    circuit.data.connect(orA, 0, andA);
    circuit.data.connect(orA, 1, andB);
    circuit.data.connect(out, 0, orA);
    return circuit;
}
int main() {
    Circuit circuit = XorGate();
    auto comp = circuit.data.getComponent(0);
    circuit.CacheLinesNeeded(comp.printSize());
    std::cout <<"True size: " << sizeof(Circuit::Component) << "\n";
    circuit.CacheLinesNeeded(sizeof(Circuit::Component));

    return 0;
}

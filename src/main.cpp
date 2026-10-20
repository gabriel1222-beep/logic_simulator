#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/format.h>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <sys/types.h>
#include <thread>
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
    In,
    Out,
    undefined,
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
    Enums::Components_enum type = Enums::Components_enum::undefined;
    Gate(Enums::Components_enum type) : in(0b00000000), type(type) {}
    std::uint64_t dest_ids[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
    std::uint8_t dest_entry[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
    Gate(uint8_t value) : in(value) {}
    std::size_t printSize() {
        std::cout << fmt::format("Type enum size = {}\n", sizeof(Gate::type));
        std::cout << fmt::format("State byte (in) size = {}\n", sizeof(Gate::in));
        std::cout << fmt::format("Dest_entry array size = {}\n", sizeof(Gate::dest_entry));
        std::cout << fmt::format("Dest_ids array size = {}\n", sizeof(Gate::dest_ids));
        std::size_t total_size = sizeof(Gate::dest_ids) + sizeof(Gate::dest_entry) + sizeof(Gate::type) + sizeof(Gate::in);
        std::cout << fmt::format("Estimated Gate instance size = {}\n", total_size);
        return total_size;
    }
    std::size_t getTrueSize() {
        return sizeof(Gate);
    }
    std::uint8_t getOutgoingConnections() {
        return (in >> 3) & 0x1F;
    }
    void setCounter(std::uint8_t value) {
        in = static_cast<std::uint8_t>(
            (in & 0b00000111) | ((value & 0b00000111) << 3));
    }
    void connect(std::uint64_t dest_id, std::uint8_t dest_entry) {
        uint8_t n = getOutgoingConnections();
        this->dest_ids[n] = dest_id;
        this->dest_entry[n] = dest_entry;
        n++;
        setCounter(n);
    }
    void evaluate() {
        using T = Enums::Components_enum;
        switch (static_cast<T>(type)) {
        case T::And:
            if (READ_BIT(in, LOGIC_GATE_A_BIT) && READ_BIT(in, LOGIC_GATE_B_BIT)) {
                turn_on(in, LOGIC_GATE_OUTPUT_BIT);
            } else {
                turn_off(in, LOGIC_GATE_OUTPUT_BIT);
            }
            return;
        case T::Not:
            set_bit(in, LOGIC_GATE_OUTPUT_BIT, !READ_BIT(in, LOGIC_GATE_A_BIT));
            return;
        case T::Or:
            if (READ_BIT(in, LOGIC_GATE_A_BIT) || READ_BIT(in, LOGIC_GATE_B_BIT)) {
                turn_on(in, LOGIC_GATE_OUTPUT_BIT);
            } else {
                turn_off(in, LOGIC_GATE_OUTPUT_BIT);
            }
            return;
        case T::Xor:
            if (READ_BIT(in, LOGIC_GATE_A_BIT) ^ READ_BIT(in, LOGIC_GATE_B_BIT)) {
                turn_on(in, LOGIC_GATE_OUTPUT_BIT);
            } else {
                turn_off(in, LOGIC_GATE_OUTPUT_BIT);
            }
            return;
        case T::ON_IN:
            turn_on(in, LOGIC_GATE_OUTPUT_BIT);
            return;
        case T::In:
            return;
        case T::Out:
            return;
        case T::Button:
            flip_bit(in, LOGIC_GATE_OUTPUT_BIT);
            return;
        case T::undefined:
            throw ::std::out_of_range("UNDEFINED COMPONENT");
        }
    }
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
    struct alignas(64) Component {
        std::uint64_t dest_ids[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
        std::uint8_t dest_entry[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
        LogicComponent component;
        Component(LogicComponent component) : component(component) {}
        void connect(std::uint64_t dest_id, std::uint8_t dest_entry) {
            std::visit([&](auto &gate) {
                uint8_t n = gate.getConnectionCounter();
                this->dest_ids[n] = dest_id;
                this->dest_entry[n] = dest_entry;
                n++;
                gate.setCounter(n);
            },
                       component);
        }
        std::uint8_t outgoingConnections() {

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
    };
    void CacheLinesNeeded(long double bytes) {
        std::cout << fmt::format("{} bytes needs {} cache lines\n", bytes, bytes / 64);
    }
    struct Data {
        std::vector<Component> Components;
        template <class type>
        std::uint64_t addComponent() {
            Components.emplace_back(LogicComponent(std::in_place_type<type>));
            return Components.size() - 1;
        }
        void connect(std::uint64_t dest_id, std::uint8_t dest_entry, std::uint64_t src_id) {
            Components.at(src_id).connect(dest_id, dest_entry);
        }
        void transferOutput(std::uint64_t dest_id, std::uint8_t dest_entry_id, const bool out) {
            auto &dest = Components[dest_id];
            std::visit([dest_entry_id, dest_id, out](auto &gate) {
                if (dest_entry_id >= gate.numberOfEntries()) throw std::out_of_range("Invalid entry id");
                set_bit(gate.in, dest_entry_id, out);
            },
                       dest.component);
        }
        Component getComponent(std::uint64_t id) {
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
        std::string getComponentType(std::size_t id) {
            if (std::holds_alternative<Out>(Components[id].component)) {
                return "Out gate";
            }
            if (std::holds_alternative<AndGate>(Components[id].component)) {
                return "AndGate";
            }
            if (std::holds_alternative<NotGate>(Components[id].component)) {
                return "NotGate";
            }
            if (std::holds_alternative<OrGate>(Components[id].component)) {
                return "OrGate";
            }
            if (std::holds_alternative<OnIn>(Components[id].component)) {
                return "OnIn";
            }
            if (std::holds_alternative<In>(Components[id].component)) {
                return "In";
            }
            return "UNDEFINED";
        }
    };
    Data data;
    void stepSimulation() {
        for (std::size_t j = 0; j < data.Components.size(); j++) {
            data.Components[j].evaluate();
            const bool output = data.Components[j].getOutput();
            // std::cout<<fmt::format("data.Components[\x1b[93m{}\x1b[0m] type = {} --- output: {}\n",j,data.getComponentType(j),output);
            //     std::cout<<fmt::format("Outgoing connections: {}\n",data.Components[j].outgoingConnections());
            for (std::size_t i = 0; i < data.Components[j].outgoingConnections(); i++) {
                data.transferOutput(data.Components[j].dest_ids[i], data.Components[j].dest_entry[i], output);
                // std::cout<<fmt::format("Connected to entry {} of {}\n",data.Components[j].dest_entry[i],data.Components[j].dest_ids[i]);
            }
        }
    }
};
Circuit XorGate() {
    Circuit circuit;
    auto andA = circuit.data.addComponent<AndGate>();
    auto andB = circuit.data.addComponent<AndGate>();
    auto orA = circuit.data.addComponent<OrGate>();
    auto notA = circuit.data.addComponent<NotGate>();
    auto notB = circuit.data.addComponent<NotGate>();
    auto inA = circuit.data.addComponent<OnIn>();
    auto inB = circuit.data.addComponent<OnIn>();
    auto out = circuit.data.addComponent<Out>();
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
    Circuit circuit;
    auto inA = circuit.data.addComponent<OnIn>();
    auto inB = circuit.data.addComponent<In>();
    auto notA = circuit.data.addComponent<NotGate>();
    auto notB = circuit.data.addComponent<NotGate>();
    auto andA = circuit.data.addComponent<AndGate>();
    auto andB = circuit.data.addComponent<AndGate>();
    auto orA = circuit.data.addComponent<OrGate>();
    auto out = circuit.data.addComponent<Out>();
    circuit.data.connect(notA, 0, inA);
    circuit.data.connect(andA, 0, notA);
    circuit.data.connect(andA, 1, inB); // AndA = notA and B
    circuit.data.connect(notB, 0, inB);
    circuit.data.connect(andB, 0, notB);
    circuit.data.connect(andB, 1, inA); // AndB = notB and A
    circuit.data.connect(orA, 0, andA);
    circuit.data.connect(orA, 1, andB);
    circuit.data.connect(out, 0, orA);

    return 0;
}

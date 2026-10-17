#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sys/types.h>
#include <variant>
#include <vector>
#include<fmt/format.h>
#define READ_BIT(byte, bit) ((((byte) >> (bit)) & 1u) != 0u)
void turn_on(std::uint8_t &byte, unsigned bit) {
    byte |= 1u << bit;
}

void turn_off(std::uint8_t &byte, unsigned bit) {
    byte &= ~(1u << bit);
}

void flip_bit(std::uint8_t &byte, unsigned bit) {
    byte ^= 1u << bit;
}

void set_bit(std::uint8_t &byte, unsigned bit, bool value) {
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
struct NotGate {
    uint8_t in = 0;

  public:
    std::uint8_t numberOfEntries() {
        return 1;
    }
    void evaluate() {
        flip_bit(in, 0);
    }
    bool getOutput() {
        return READ_BIT(in, 0);
    }
};
struct AndGate {
    uint8_t in = 0;

    std::uint8_t numberOfEntries() {
        return 2;
    }
    void evaluate() {
        if (READ_BIT(in, 0) && READ_BIT(in, 1)) {
            turn_on(in, 3);
        } else {
            turn_off(in, 3);
        }
    }
    bool getOutput() {
        return READ_BIT(in, 3);
    }
};
struct OrGate {
    uint8_t in = 0;

    std::uint8_t numberOfEntries() {
        return 2;
    }
    void evaluate() {
        if (READ_BIT(in, 0) || READ_BIT(in, 1)) {
            turn_on(in, 3);
        } else {
            turn_off(in, 3);
        }
    }
    bool getOutput() {
        return READ_BIT(in, 3);
    }
};
struct XorGate {
    uint8_t in = 0;

    std::uint8_t numberOfEntries() {
        return 2;
    }
    void evaluate() {
        if (READ_BIT(in, 0) ^ READ_BIT(in, 1)) {
            turn_on(in, 3);
        } else {
            turn_off(in, 3);
        }
    }
    bool getOutput() {
        return READ_BIT(in, 3);
    }
};
struct Button {
    uint8_t in = 0;

    std::uint8_t numberOfEntries() {
        return 0;
    }
    void evaluate() {
        flip_bit(in, 0);
    }
    bool getOutput() {
        return READ_BIT(in, 3);
    }
};
struct OnIn {
    uint8_t in = 0;
    std::uint8_t numberOfEntries() {
        return 0;
    }
    void evaluate() {}
    bool getOutput() {
        return 1;
    }
};
struct Circuit {
    using LogicComponent = std::variant<AndGate,
                                        OrGate,
                                        NotGate,
                                        XorGate,
                                        OnIn,
                                        Button>;
    using id = std::uint32_t;
    struct alignas(64) Component {
        LogicComponent component;
        id dest_id[24];
        std::uint8_t dest_entry_id[24];
        std::uint8_t n = 0;
        Component(LogicComponent component) : component(component){}
        void connect(id dest_id, std::uint8_t dest_entry) {
            this->dest_id[n] = dest_id;
            this->dest_entry_id[n] = dest_entry;
            n++;
        }
    };
        std::size_t printSize(){
            std::cout << fmt::format("LogicComponent size = {}\n",sizeof(Component::component));
            std::cout << fmt::format("Dest_entry_id size = {}\n",sizeof(Component::dest_entry_id));
            std::cout << fmt::format("Dest_id size = {}\n",sizeof(Component::dest_id));
            std::cout << fmt::format("n size = {}\n",sizeof(Component::n));
            std::cout << fmt::format("Estimated size = {}\n",sizeof(Component::n)+sizeof(Component::dest_id)+sizeof(Component::dest_entry_id)+sizeof(Component::component));
            return sizeof(Component::n)+sizeof(Component::dest_id)+sizeof(Component::dest_entry_id)+sizeof(Component::component);
        }
        void CacheLinesNeeded(long double bytes){
            std::cout << fmt::format("{} bytes needs {} cache lines\n",bytes,bytes/64);
        }
    std::vector<Component> Components;
    bool getOutput(LogicComponent &comp) {
        return std::visit(
            [](auto &gate) { return gate.getOutput(); },
            comp);
    }
    void evaluate(LogicComponent &comp) {
        std::visit(
            [](auto &gate) { gate.evaluate(); },
            comp);
    }
    bool getOutput(id id) {
        return std::visit(
            [](auto &gate) { return gate.getOutput(); },
            Components[id].component);
    }
    void processConnection(id dest_id,std::uint8_t dest_entry_id,const bool out) {
        auto &dest = Components[dest_id];
        std::visit([dest_entry_id,dest_id, out](auto &gate) {
            if (dest_entry_id >= gate.numberOfEntries()) throw std::out_of_range("Invalid entry id");
            set_bit(gate.in, dest_entry_id, out);
        },
                   dest.component);
    }
    template <class type>
    id addComponent() {
        Components.emplace_back(LogicComponent(std::in_place_type<type>));
        return Components.size() - 1;
    }
    void connect(id dest_id, std::uint8_t dest_entry, id src_id) {
        Components.at(src_id).connect(dest_id, dest_entry);
    }
    void step(std::size_t steps) {
        repeat_fwrd(steps, [&](std::size_t) {
            for (auto &component : Components) {
                evaluate(component.component);
                const bool output = getOutput(component.component);
                for(std::size_t i = 0; i<component.n;i++){
                    processConnection(component.dest_id[i],component.dest_entry_id[i], output);
                }
            }
        });
    }
};
int main() {
    Circuit circuit;
    auto size = circuit.printSize();
    circuit.CacheLinesNeeded(size);
    std::cout <<"Actual size = " <<sizeof(Circuit::Component)<<"\n";
    circuit.CacheLinesNeeded(sizeof(Circuit::Component));
    auto andAId = circuit.addComponent<AndGate>();
    auto andBId = circuit.addComponent<AndGate>();
    auto onInId = circuit.addComponent<OnIn>();
    auto notGId = circuit.addComponent<NotGate>();
    circuit.connect(andBId, 1, onInId);
    circuit.connect(andBId, 0, onInId);
    circuit.connect(andAId, 0, andBId);
    circuit.connect(andAId, 1, andBId);
    circuit.connect(notGId, 0, andAId);
    circuit.step(1000);
    bool GateAOut = circuit.getOutput(andAId);
    bool GateBOut = circuit.getOutput(andBId);
    bool NotGateOut = circuit.getOutput(notGId);
    std::cout << GateAOut << std::endl;
    std::cout << GateBOut << std::endl;
    std::cout << NotGateOut << std::endl;

    return 0;
}

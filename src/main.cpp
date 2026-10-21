#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
#define READ_BIT(byte, bit) ((((byte) >> (bit)) & 1u) != 0u)
template <typename T, typename... Args>
constexpr bool matches_any(const T& target, const Args&... items) {
    return ((target == items) || ...);
}
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
    Gate(Enums::Components_enum type) : in(0b00000000){}
    std::uint64_t dest_ids[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
    std::uint8_t dest_entry[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
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
    std::uint8_t OutgoingConnections() {
        return (in >> 3) & 0x1F;
    }
    void setCounter(std::uint8_t value) {
        in = static_cast<std::uint8_t>(
            (in & 0b00000111) | ((value & 0b00000111) << 3));
    }
    std::uint8_t numberOfEntries(){
        using T = Enums::Components_enum;
        switch (static_cast<T>(type)) {
        case T::And:
            return 2;
        case T::Not:
            return 1;
        case T::Or:
            return 2;
        case T::Xor:
            return 2;
        case T::ON_IN:
            return 0;
        case T::In:
            return 1;
        case T::Out:
            return 1;
        case T::Button:
            return 0;
        case T::undefined:
            throw ::std::out_of_range("UNDEFINED COMPONENT");
        }
        
    }
    void connect(std::uint64_t dest_id, std::uint8_t dest_entry) {
        uint8_t n = OutgoingConnections();
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
    void CacheLinesNeeded(long double bytes) {
        std::cout << fmt::format("{} bytes needs {} cache lines\n", bytes, bytes / 64);
    }
    struct Data {
        std::vector<Gate> Components;
        template <Enums::Components_enum type>
        std::uint64_t addComponent() {
            Components.emplace_back(type);
            return Components.size() - 1;
        }
        void connect(std::uint64_t dest_id, std::uint8_t dest_entry, std::uint64_t src_id) {
            Components.at(src_id).connect(dest_id, dest_entry);
        }
        void transferOutput(std::uint64_t dest_id, std::uint8_t dest_entry_id, const bool out) {
            Gate &dest = Components[dest_id];
            if (dest_entry_id >= dest.numberOfEntries()) throw std::out_of_range("Invalid entry id");
            set_bit(dest.in, dest_entry_id, out);
        }
        std::size_t getinputNumber() {
            auto ins = 0;
            for (auto &component : Components) {
                if(component.type == Enums::Components_enum::In) ins++;
            }
            return ins;
        }
        std::size_t getoutputNumber() {
            auto outs = 0;
            for (auto &component : Components) {
                if(component.type == Enums::Components_enum::Out) outs++;
            }
            return outs;
        }
    };
    Data data;
    void stepSimulation() {
        for (std::size_t j = 0; j < data.Components.size(); j++) {
            auto &component = data.Components[j];
            component.evaluate();
            const bool output = component.getOutput();
            for (std::size_t i = 0; i < component.OutgoingConnections(); i++) {
                data.transferOutput(component.dest_ids[i], component.dest_entry[i], output);
            }
        }
    }
};
struct Position{
    uint16_t x = 0;
    uint16_t y = 0;
    Position(uint16_t x, uint16_t y) : x(x),y(y){}
};
struct Camera{
  Position pos;
  Camera(Position pos) : pos(pos){}
  void moveRight(){
      pos.x++;
  }  
  void moveLeft(){
      pos.x--;
  }  
  void moveUp(){
      pos.y--;
  }
  void moveDown(){
      pos.y++;
  }
};
struct GUI{
    
};
int main() {
    Circuit circuit;
    auto andA = circuit.data.addComponent<Enums::Components_enum::And>();
    auto andB = circuit.data.addComponent<Enums::Components_enum::And>();
    auto orA = circuit.data.addComponent<Enums::Components_enum::Or>();
    auto notA = circuit.data.addComponent<Enums::Components_enum::Not>();
    auto notB = circuit.data.addComponent<Enums::Components_enum::Not>();
    auto inA = circuit.data.addComponent<Enums::Components_enum::ON_IN>();
    auto inB = circuit.data.addComponent<Enums::Components_enum::ON_IN>();
    auto out = circuit.data.addComponent<Enums::Components_enum::Out>();
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

#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "boolArray.hpp"

namespace Enums {
enum Components_enum : std::uint8_t {
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

#define READ_BIT(value, pos) ((((value) >> (pos)) & 1u) != 0u)

#define LOGIC_GATE_OUTPUT_BIT 2
#define LOGIC_GATE_OUTPUT_ON 0b00000100
#define LOGIC_GATE_A_BIT 0
#define LOGIC_GATE_B_BIT 1
#define LOGIC_GATE_MAX_OUTGOING_CONNECTIONS 14

#define NUMBER_OF_COMPONENTS (static_cast<std::uint8_t>(Enums::Components_enum::undefined) - 1)

constexpr std::array<std::uint8_t, 9> EntryCountTable = {
    2, // 0: And
    2, // 1: Or
    1, // 2: Not
    2, // 3: Xor
    0, // 4: ON_IN
    0, // 5: Button ()
    1, // 6: In
    1, // 7: Out
    0  // 8: undefined
};

bool SameComponent(
    Enums::Components_enum component_typeA,
    Enums::Components_enum component_typeB);

struct Gate {
    std::uint8_t dest_ids[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS]{};
    BoolArray<LOGIC_GATE_MAX_OUTGOING_CONNECTIONS,Mode::fast> dest_entry;
    std::uint8_t in = 0b00000000;
    const Enums::Components_enum type = Enums::Components_enum::undefined;
    constexpr Gate(Enums::Components_enum type)
        : in(0b00000000), type(type) {
    }

    std::size_t printSize() const;
    std::size_t getTrueSize() const;

    std::uint8_t OutgoingConnections() const {
        return (in >> 3) & 0b00011111;

    }
    void setCounter(std::uint8_t value);

    void connect(std::uint64_t dest_id, std::uint8_t dest_entry);

  private:
    static bool And(std::uint8_t in) {
        return (in & 0b00000011) == 0b00000011;
    }

    static bool Or(std::uint8_t in) {
        return (in & 0b00000011) != 0;
    }

    static bool Not(std::uint8_t in) {
        return (in & 0b00000001) == 0;
    }

    static bool Xor(std::uint8_t in) {
        return ((in & 0b00000001) ^ ((in & 0b00000010) >> 1)) != 0;
    }

    static bool OnIn(std::uint8_t) {
        return true;
    }

    static bool Button(std::uint8_t in) {
        return (in & 0b00000100) == 0;
    }

    static bool In(std::uint8_t in) {
        return (in & 0b00000100) != 0;
    }

    static bool Out(std::uint8_t in) {
        return (in & 0b00000100) != 0;
    }

    static bool Undefined(std::uint8_t) {
        throw std::out_of_range("UNDEFINED COMPONENT");
    }


  public:
    static constexpr std::array<bool (*)(std::uint8_t), 9> OperationTable = {
        And, Or, Not, Xor, OnIn, Button, In, Out, Undefined};
    inline void evaluateTable() {
        if (type == Enums::ON_IN) {
            in = 0b00000100;
            return;
        }
        in = static_cast<std::uint8_t>(
            (in & ~0b00000100u) |
            (static_cast<std::uint8_t>(OperationTable[type](in)) << 2));
    }
   inline  std::uint8_t numberOfEntriesTable() const {
        if (type == Enums::ON_IN) {
            return 0;
        }
        return EntryCountTable[static_cast<std::size_t>(type)];
    }

    inline bool getOutput() {
        return (in & 0b00000100) != 0;
    }


    constexpr void setInput(std::uint8_t entry, bool value) {
        this->in = static_cast<std::uint8_t>(
            (this->in & ~(1u << entry)) |
            (-static_cast<std::uint8_t>(value) & (1u << entry)));
    }
};

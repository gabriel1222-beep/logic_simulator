#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <concepts>
#include <stdexcept>
#include "BitWise.hpp"

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
    Enums::Components_enum component_typeB
);

struct Gate {
    std::uint64_t dest_ids[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
    std::uint8_t dest_entry[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
    std::uint8_t in = 0b00000000;
    Enums::Components_enum type = Enums::Components_enum::undefined;

    Gate(Enums::Components_enum type);

    std::size_t printSize() const;
    std::size_t getTrueSize() const;

    std::uint8_t OutgoingConnections() const;
    void setCounter(std::uint8_t value);

    std::uint8_t numberOfEntriesTable() const;
    std::uint8_t numberOfEntriesif() const;

    void connect(std::uint64_t dest_id, std::uint8_t dest_entry);

    const std::array<bool (*)(const std::uint8_t), 9> OperationTable = {
        [](const std::uint8_t in) {
            return BitWise::read_bit<LOGIC_GATE_A_BIT>(in) &&
                   BitWise::read_bit<LOGIC_GATE_B_BIT>(in);
        }, // 0: And

        [](const std::uint8_t in) {
            return BitWise::read_bit<LOGIC_GATE_A_BIT>(in) ||
                   BitWise::read_bit<LOGIC_GATE_B_BIT>(in);
        }, // 1: Or

        [](const std::uint8_t in) {
            return !BitWise::read_bit<LOGIC_GATE_A_BIT>(in);
        }, // 2: Not

        [](const std::uint8_t in) {
            return (BitWise::read_bit<LOGIC_GATE_A_BIT>(in) !=
                    BitWise::read_bit<LOGIC_GATE_B_BIT>(in));
        }, // 3: Xor

        [](const std::uint8_t) {
            return true;
        }, // 4: ON_IN

        [](const std::uint8_t in) {
            return !BitWise::read_bit<LOGIC_GATE_OUTPUT_BIT>(in);
        }, // 5: Button (toggle output)

        [](const std::uint8_t in) {
            return BitWise::read_bit<LOGIC_GATE_OUTPUT_BIT>(in);
        }, // 6: In (preserve output)

        [](const std::uint8_t in) {
            return BitWise::read_bit<LOGIC_GATE_OUTPUT_BIT>(in);
        }, // 7: Out (preserve output)

        [](const std::uint8_t) -> bool {
            throw std::out_of_range("UNDEFINED COMPONENT");
        } // 8: undefined
    };

    void evaluateTable();
    void evaluateSwitch();

    bool getOutput();

    template <class F>
        requires std::invocable<F&, std::uint64_t, std::uint8_t>
    void forEachOutgoingConnection(F&& f) const {
        const auto count = OutgoingConnections();

        for (std::size_t i = 0; i < count; ++i) {
            f(dest_ids[i], dest_entry[i]);
        }
    }

    void setInput(std::uint8_t entry, bool value);
};


#include <fmt/format.h>

#include "BitWise.hpp"
#include "gates.hpp"
#include "helper.hpp"
#include <iostream>

bool SameComponent(
    Enums::Components_enum component_typeA,
    Enums::Components_enum component_typeB
) {
    return (component_typeA == component_typeB) ? true : false;
}

Gate::Gate(Enums::Components_enum type)
    : in(0b00000000), type(type) {
}

std::size_t Gate::printSize() const {
    std::cout << fmt::format("Type enum size = {}\n", sizeof(Gate::type));
    std::cout << fmt::format("State byte (in) size = {}\n", sizeof(Gate::in));
    std::cout << fmt::format("Dest_entry array size = {}\n", sizeof(Gate::dest_entry));
    std::cout << fmt::format("Dest_ids array size = {}\n", sizeof(Gate::dest_ids));

    std::size_t total_size =
        sizeof(Gate::dest_ids) +
        sizeof(Gate::dest_entry) +
        sizeof(Gate::type) +
        sizeof(Gate::in);

    std::cout << fmt::format("Estimated Gate instance size = {}\n", total_size);

    return total_size;
}

std::size_t Gate::getTrueSize() const {
    return sizeof(Gate);
}

std::uint8_t Gate::OutgoingConnections() const {
    return BitWise::getField<3, 5>(in);
}

void Gate::setCounter(std::uint8_t value) {
    in = BitWise::setField<3, 5>(in, value);
}

std::uint8_t Gate::numberOfEntriesTable() const {
    return EntryCountTable[static_cast<std::size_t>(type)];
}

std::uint8_t Gate::numberOfEntriesif() const {

    if (matchesAny(type, Enums::Components_enum::And, Enums::Components_enum::Or, Enums::Components_enum::Xor)) return 2;
    if (matchesAny(type, Enums::Components_enum::Not, Enums::Components_enum::In, Enums::Components_enum::Out)) return 1;

    return 0;
}

void Gate::connect(std::uint64_t dest_id, std::uint8_t dest_entry) {
    std::uint8_t n = OutgoingConnections();

    this->dest_ids[n] = dest_id;
    this->dest_entry[n] = dest_entry;

    n++;
    setCounter(n);
}

void Gate::evaluateTable() {
    in = BitWise::set_bit<LOGIC_GATE_OUTPUT_BIT>(in, OperationTable[type](in));
}

void Gate::evaluateSwitch() {
    using T = Enums::Components_enum;

    bool output;

    switch (type) {
    case T::And:
        output = BitWise::read_bit<LOGIC_GATE_A_BIT>(in) &&
                 BitWise::read_bit<LOGIC_GATE_B_BIT>(in);
        break;

    case T::Or:
        output = BitWise::read_bit<LOGIC_GATE_A_BIT>(in) ||
                 BitWise::read_bit<LOGIC_GATE_B_BIT>(in);
        break;

    case T::Not:
        output = !BitWise::read_bit<LOGIC_GATE_A_BIT>(in);
        break;

    case T::Xor:
        output = BitWise::read_bit<LOGIC_GATE_A_BIT>(in) !=
                 BitWise::read_bit<LOGIC_GATE_B_BIT>(in);
        break;

    case T::ON_IN:
        output = true;
        break;

    case T::Button:
        output = !BitWise::read_bit<LOGIC_GATE_OUTPUT_BIT>(in);
        break;

    case T::In:
    case T::Out:
        return; // Preserve the byte.

    default:
        throw std::out_of_range("UNDEFINED COMPONENT");
    }

    in = BitWise::set_bit<LOGIC_GATE_OUTPUT_BIT>(in, output);
}

bool Gate::getOutput() {
    return BitWise::read_bit<LOGIC_GATE_OUTPUT_BIT>(in);
}

void Gate::setInput(std::uint8_t entry, bool value) {
    const auto mask = static_cast<std::uint8_t>(1u << entry);

    if (value)
        this->in |= mask;
    else
        this->in &= static_cast<std::uint8_t>(~mask);
}

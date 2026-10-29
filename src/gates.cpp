
#include <fmt/format.h>

#include "BitWise.hpp"
#include "gates.hpp"
#include <iostream>

bool SameComponent(
    Enums::Components_enum component_typeA,
    Enums::Components_enum component_typeB
) {
    return (component_typeA == component_typeB) ? true : false;
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


void Gate::setCounter(std::uint8_t value) {
    in = BitWise::setField<3, 5>(in, value);
}



void Gate::connect(std::uint64_t dest_id, std::uint8_t dest_entry) {
    std::uint8_t n = OutgoingConnections();

    this->dest_ids[n] = dest_id;
    this->dest_entry[n] = dest_entry;

    n++;
    setCounter(n);
}





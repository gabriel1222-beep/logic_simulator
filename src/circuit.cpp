#include "circuit.hpp"

#include <iostream>

#include <fmt/format.h>

void Circuit::CacheLinesNeeded(long double bytes) {
    std::cout << fmt::format(
        "{} bytes needs {} cache lines\n",
        bytes,
        bytes / 64
    );
}

void Circuit::Data::connect(
    std::uint64_t dest_component_id,
    std::uint8_t dest_component_entry,
    std::uint64_t src_component_id
) {
    LogicComponents.at(src_component_id).connect(
        dest_component_id,
        dest_component_entry
    );
}

std::size_t Circuit::Data::inputsInCircuit() {
    auto ins = 0;

    for (auto& component : LogicComponents) {
        if (component.type == Enums::Components_enum::In)
            ins++;
    }

    return ins;
}

std::size_t Circuit::Data::outputsInCircuit() {
    auto outs = 0;

    for (auto& component : LogicComponents) {
        if (component.type == Enums::Components_enum::Out)
            outs++;
    }

    return outs;
}

void Circuit::stepSimulationTable() {
    for (auto& component : data.LogicComponents) {
        component.evaluateTable();

        const bool output = component.getOutput();

        component.forEachOutgoingConnection(
            [&](auto destination_id, auto input_entry) {
                Gate& dest = data.LogicComponents[destination_id];

                if (input_entry >= dest.numberOfEntriesTable())
                    throw std::out_of_range("Invalid entry id");

                dest.setInput(input_entry, output);
            }
        );
    }
}

void Circuit::stepSimulationSwitch() {
    for (auto& component : data.LogicComponents) {
        component.evaluateSwitch();

        const bool output = component.getOutput();

        component.forEachOutgoingConnection(
            [&](auto destination_id, auto input_entry) {
                Gate& dest = data.LogicComponents[destination_id];

                if (input_entry >= dest.numberOfEntriesif())
                    throw std::out_of_range("Invalid entry id");

                dest.setInput(input_entry, output);
            }
        );
    }
}

#pragma once

#include "gates.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

struct Circuit {
    void CacheLinesNeeded(long double bytes);

    struct Data {
        std::vector<Gate> LogicComponents;

        template <Enums::Components_enum type>
        std::uint64_t addComponent() {
            LogicComponents.emplace_back(type);
            return LogicComponents.size() - 1;
        }

        void connect(
            std::uint64_t dest_component_id,
            std::uint8_t dest_component_entry,
            std::uint64_t src_component_id
        );

        std::size_t inputsInCircuit();
        std::size_t outputsInCircuit();
    };

    Data data;

    void stepSimulationTable();
    void stepSimulationSwitch();
};

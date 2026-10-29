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
        constexpr std::uint64_t addComponent() {
            LogicComponents.emplace_back(type);
            return LogicComponents.size() - 1;
        }

        void connect(
            std::uint64_t dest_component_id,
            std::uint8_t dest_component_entry,
            std::uint64_t src_component_id) {
            LogicComponents.at(src_component_id).connect(dest_component_id, dest_component_entry);
        }

        std::size_t inputsInCircuit() {
            auto ins = 0;

            for (auto &component : LogicComponents) {
                if (component.type == Enums::Components_enum::In)
                    ins++;
            }

            return ins;
        }

        std::size_t outputsInCircuit() {
            auto outs = 0;

            for (auto &component : LogicComponents) {
                if (component.type == Enums::Components_enum::Out)
                    outs++;
            }

            return outs;
        }
    };

    Data data;
    void stepSimulationTable() {
        for (auto &component : data.LogicComponents) {
            component.evaluateTable();

            const bool output = component.getOutput();
            const auto count = component.OutgoingConnections();

            for (std::size_t i = 0; i < count; ++i) {
                data.LogicComponents[component.dest_ids[i]].setInput(component.dest_entry[i], output);
            }
        }
    }
};

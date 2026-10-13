#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <sys/types.h>
#include <type_traits>
#include <variant>
#include <vector>

#define BIT_ON(value, bit) (((value) & (std::uint8_t{1} << bit)) != 0)
template <const std::uint8_t bit>
constexpr bool bitOn(const std::uint8_t &value) {
    static_assert(bit < 8, "Bit must be less than 8");
    return ((value & (std::uint8_t{1} << bit)) != 0);
}

constexpr void setBit(std::uint8_t &value, const std::uint8_t bit) {
    if (bit < 8) {
        value ^= std::uint8_t{1} << bit;
    }
}

constexpr void clearBit(std::uint8_t &value, const std::uint8_t bit) {
    if (bit < 8) {
        value &= static_cast<std::uint8_t>(~(std::uint8_t{1} << bit));
    }
}
template <auto typeA, auto typeB>
constexpr bool EqualType() {
    return std::is_same_v<decltype(typeA), decltype(typeB)>;
}
#define LOGIC_GATE_COMPONENTS(X) \
    X(And)                       \
    X(Or)                        \
    X(Not)
#define OTHER_COMPONENTS(X) \
    X(OnInput)
enum class Component_type_ids : std::uint8_t {
#define X(component) component,
    LOGIC_GATE_COMPONENTS(X)
#undef X
#define X(component) component,
        OTHER_COMPONENTS(X)
#undef X
};
namespace Components {
namespace LogicGates {
#define X(component) inline constexpr Component_type_ids component = Component_type_ids::component;
LOGIC_GATE_COMPONENTS(X)
#undef X
} // namespace LogicGates

namespace other {
#define X(component) inline constexpr Component_type_ids component = Component_type_ids::component;
OTHER_COMPONENTS(X)
#undef X
} // namespace other
template <Component_type_ids id>
constexpr bool component_exists() {
#define X(component)                                     \
    if constexpr (id == Component_type_ids::component) { \
        return 1;                                        \
    }
    LOGIC_GATE_COMPONENTS(X)

#undef X
#define X(component)                                     \
    if constexpr (id == Component_type_ids::component) { \
        return 1;                                        \
    }
    OTHER_COMPONENTS(X)

#undef X
    return 0;
}
} // namespace Components
struct Circuit {
  private:
    unsigned long int ids = 0;

  public:
    template <std::uint8_t number_of_inputs, std::uint8_t number_of_outputs>
    struct Gate_ID {
        static_assert(number_of_inputs > 0 && number_of_outputs > 0,
                      "Invalid identification");
        std::uint32_t id;
        std::uint32_t entries_ids[number_of_inputs] = {0};
        std::uint32_t output_ids[number_of_outputs] = {0};
    };
    struct AndGate {
        std::uint8_t entries = 0;
        Gate_ID<2, 1> gate_id;
        bool getOut() {
            return (BIT_ON(entries, 0) && BIT_ON(entries, 1)) ? 1 : 0;
        }
    };
    struct OrGate {
        std::uint8_t entries = 0;
        Gate_ID<2, 1> gate_id;
        bool getOut() {
            return (BIT_ON(entries, 0) || BIT_ON(entries, 1)) ? 1 : 0;
        }
    };
    struct NotGate {
        std::uint8_t entry = 0;
        Gate_ID<1, 1> gate_id;
        bool getOut() { return !entry; }
    };
    struct OnInput {
        std::uint32_t id, out_id;
        bool getOut() { return 1; }
    };

  private:
    std::vector<AndGate> AndGates;
    std::vector<NotGate> NotGates;
    std::vector<OrGate> OrGates;
    std::vector<OnInput> OnInputs;
    using component_ptrs_t = std::variant<AndGate *, NotGate *, OrGate *, OnInput *, std::nullptr_t>;
    template <Component_type_ids component>
    component_ptrs_t getComponent(std::uint32_t id) {
        static_assert(Components::component_exists<component>(), "\n\ngetComponent(): Invalid component\n\n");
        switch (component) {
        case Components::LogicGates::And:
            for (auto &andGate : AndGates)
                if (andGate.gate_id.id == id) return &andGate;
            break;
        case Components::LogicGates::Or:
            for (auto &orGate : OrGates)
                if (orGate.gate_id.id == id) return &orGate;
            break;
        case Components::LogicGates::Not:
            for (auto &notGate : NotGates)
                if (notGate.gate_id.id == id) return &notGate;
            break;
        case Components::other::OnInput:
            for (auto &onInput : OnInputs)
                if (onInput.id == id) return &onInput;
            break;
        }

        return nullptr;
    }

  public:
    std::uint32_t newComponent(const Component_type_ids gate_type) {
        switch (gate_type) {
        case Components::LogicGates::Or: {
            OrGate gate;
            gate.gate_id.id = ids;
            OrGates.push_back(gate);
        } break;
        case Components::LogicGates::And: {
            AndGate gate;
            gate.gate_id.id = ids;
            AndGates.push_back(gate);
        }
        case Components::LogicGates::Not: {
            NotGate gate;
            gate.gate_id.id = ids;
            NotGates.push_back(gate);
            break;
        }
        case Components::other::OnInput:
            OnInput in;
            in.id = ids;
            OnInputs.push_back(in);
        }
        ids++;
        return ids - 1;
    }
    template <auto dest_component, auto src_component>
    void connect(std::uint32_t dest_id, std::uint32_t src_id,
                 std::uint8_t dest_entry) {
        static_assert(component_exists(dest_component), "Circuit::Connect(): invalid dest_component");
        static_assert(component_exists(src_component), "Circuit::Connect(): invalid src_component");
        auto dest = getComponent<dest_component>(dest_id);
        auto src = getComponent<src_component>(src_id);
        if constexpr (dest_component == Components::LogicGates::And) {
            if (AndGate *gate = std::get_if<AndGate *>(&dest); gate != nullptr) {
                
            }
        }
    }
};

int main() { return 0; }

#include <array>
#include <cstdint>
#include <variant>
#include <vector>

constexpr bool bitOn(std::uint8_t value, unsigned int bit) {
  return bit < 8 && ((value & (std::uint8_t{1} << bit)) != 0);
}

constexpr void setBit(std::uint8_t &value, unsigned int bit) {
  if (bit < 8) {
    value |= std::uint8_t{1} << bit;
  }
}

constexpr void clearBit(std::uint8_t &value, unsigned int bit) {
  if (bit < 8) {
    value &= static_cast<std::uint8_t>(~(std::uint8_t{1} << bit));
  }
}
enum class LogicGateType : std::uint8_t { And, Or, Not };
struct LogicGate {
  LogicGateType type;
  std::uint32_t id;
  std::array<std::uint32_t, 2> inputs = {0};
  std::uint8_t inputCount;
  bool output = false;
};
struct Button {
  bool on = false;
  std::uint32_t id;
  std::array<std::uint32_t, 1> out{};
  void set() { on = !on; }
};
struct LogicCircuit {
  using gateId = std::uint32_t;
  using buttonId = std::uint32_t;
  using Component = std::variant<LogicGate, Button>;

private:
  std::vector<Component> components;
  Component *getComponent(std::uint32_t id) {
    for (auto &component : components) {
      const bool matches = std::visit(
          [id](const auto &item) { return item.id == id; }, component);

      if (matches) {
        return &component;
      }
    }
    return nullptr;
  }
  bool ifLogicGate(const Component &component) {
    return (std::holds_alternative<LogicGate>(component)) ? true : false;
  }
  bool ifButton(const Component &component) {
    return (std::holds_alternative<Button>(component)) ? true : false;
  }

public:
  gateId addGate(LogicGateType type) {
    LogicGate gate;
    gate.inputCount = (type == LogicGateType::Not) ? 1 : 2;
    gate.type = type;
    gate.id = static_cast<std::uint32_t>(components.size() + 1);
    components.push_back(gate);
    return gate.id;
  }
  buttonId addButton() {
    Button button;
    button.id = static_cast<std::uint32_t>(components.size() + 1);
    components.push_back(button);
    return button.id;
  }
  void setbutton(buttonId id) {
    Component *component = getComponent(id);
    if (auto *button = std::get_if<Button>(component)) {
      button->set();
    }
  }
  void connectComponent(const std::uint32_t dest_id, const std::uint32_t src_id,
                        bool dest_in) {
    Component *component = getComponent(dest_id);

    if (auto *gate = std::get_if<LogicGate>(component)) {
      gate->inputs[dest_in] = src_id;
    }
  }
  void step() {
    for (auto &component : components) {
    }
  }
};
int main() {
  LogicCircuit circuit;
  LogicCircuit::gateId andGate_id = circuit.addGate(LogicGateType::And);
  LogicCircuit::buttonId buttonA_id = circuit.addButton();
  LogicCircuit::buttonId buttonB_id = circuit.addButton();
  circuit.connectComponent(andGate_id, buttonA_id, 0);
  circuit.connectComponent(andGate_id, buttonB_id, 1);
  circuit.setbutton(buttonA_id);
  return 0;
}

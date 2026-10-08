#include <cstdint>
#include <array>
#include <list>
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
enum class LogicGateType : std::uint8_t{
  And,Or,Not
};
struct LogicGate{
  LogicGateType type;
  std::uint32_t id;
  std::array<std::uint32_t, 2> inputs = {0};
  std::uint8_t inputCount;
  bool output = false;
};
struct Button{
  bool on = false;
  std::uint32_t id;
  std::array<std::uint32_t, 1> out{};
  void set(){
    on = !on;
  }
};
struct Components{
  
};
struct LogicCircuit{
  using gateId = std::uint32_t;
  using buttonId = std::uint32_t;
  private:
    std::vector<LogicGate> Gates;
    std::vector<Button> Buttons;
    LogicGate *getGate(gateId id){
      for(long unsigned int i = 0 ; i< Gates.size();i++){
        if(Gates[i].id == id) return &Gates[i];
      }
      return NULL;
    }
    Button *getButton(buttonId id){
      for(long unsigned int i = 0 ; i< Buttons.size();i++){
        if(Buttons[i].id == id) return &Buttons[i];
      }
      return NULL;
    }
  public:
    gateId addGate(LogicGateType type){
      LogicGate gate;
      gate.inputCount = (type == LogicGateType::Not) ? 1 : 2;
      gate.type = type;
      gate.id = static_cast<std::uint32_t>(Gates.size()+1);
      Gates.push_back(gate);
      return gate.id;
    }
    buttonId addButton(){
      Button button;
      button.id = static_cast<std::uint32_t>(Buttons.size()+1);
      Buttons.push_back(button);
      return button.id;
    }
    void setbutton(buttonId id){
      Button *b = getButton(id);
      b->set();
    }
    void connectLogicGates(const gateId dest_id,const gateId src_id,bool dest_in){
      LogicGate* dest = getGate(dest_id);
      dest->inputs[dest_in] = src_id;
    }
    void connectButtonToLogicGate(const gateId dest_id,buttonId button_id,bool dest_in){
      LogicGate* dest = getGate(dest_id);
      dest->inputs[dest_in] = button_id; 
    }
    void step(){
       
    }
};
int main() {
  LogicCircuit circuit;
  LogicCircuit::gateId andGate_id = circuit.addGate(LogicGateType::And);
  LogicCircuit::buttonId buttonA_id = circuit.addButton();
  LogicCircuit::buttonId buttonB_id = circuit.addButton();
  circuit.connectButtonToLogicGate(andGate_id, buttonA_id, 0);
  circuit.connectButtonToLogicGate(andGate_id, buttonB_id, 1);
  circuit.setbutton(buttonA_id);
  return 0;
}

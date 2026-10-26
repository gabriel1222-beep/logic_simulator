#include "BitWise.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL_error.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <thread>
#include <vector>
template <typename T, typename... Args>
constexpr bool matches_any(const T &target, const Args &...items) {
    return ((target == items) || ...);
}
void exitError(std::string Error) {
    std::cout << "[\x1b[93mFATAL ERROR\x1b[0m]" << Error << std::endl;
    exit(EXIT_FAILURE);
}

namespace Enums {
enum class Components_enum : std::uint8_t {
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
#define LOGIC_GATE_OUTPUT_BIT 2
#define LOGIC_GATE_OUTPUT_ON 0b00000100
#define LOGIC_GATE_A_BIT 0
#define LOGIC_GATE_B_BIT 1
#define LOGIC_GATE_MAX_OUTGOING_CONNECTIONS 14
} // namespace Enums
#define NUMBER_OF_COMPONENTS (static_cast<uint8_t>(Enums::Components_enum::undefined) - 1)
bool SameComponent(Enums::Components_enum component_typeA, Enums::Components_enum component_typeB) {
    return (component_typeA == component_typeB) ? true : false;
}
struct Gate {
    std::uint64_t dest_ids[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
    std::uint8_t dest_entry[LOGIC_GATE_MAX_OUTGOING_CONNECTIONS];
    uint8_t in = 0b00000000;
    Enums::Components_enum type = Enums::Components_enum::undefined;
    constexpr Gate(Enums::Components_enum type) : in(0b00000000), type(type) {}
    std::size_t printSize() const {
        std::cout << fmt::format("Type enum size = {}\n", sizeof(Gate::type));
        std::cout << fmt::format("State byte (in) size = {}\n", sizeof(Gate::in));
        std::cout << fmt::format("Dest_entry array size = {}\n", sizeof(Gate::dest_entry));
        std::cout << fmt::format("Dest_ids array size = {}\n", sizeof(Gate::dest_ids));
        std::size_t total_size = sizeof(Gate::dest_ids) + sizeof(Gate::dest_entry) + sizeof(Gate::type) + sizeof(Gate::in);
        std::cout << fmt::format("Estimated Gate instance size = {}\n", total_size);
        return total_size;
    }
    std::size_t getTrueSize() const {
        return sizeof(Gate);
    }
    std::uint8_t OutgoingConnections() const {
        return BitWise::getField<3, 5>(in);
    }
    void setCounter(std::uint8_t value) {
        in = BitWise::setField<3, 5>(in, value);
    }
    static constexpr std::array<std::uint8_t, 9> EntryCountTable = {
        2, // 0: And
        2, // 1: Or
        1, // 2: Not
        2, // 3: Xor
        0, // 4: ON_IN 
        0, // 5: Button 
        1, // 6: In
        1, // 7: Out
        0  // 8: undefined 
    };
    constexpr std::uint8_t numberOfEntries() const {
        return EntryCountTable[static_cast<std::size_t>(type)];
    }
    void connect(std::uint64_t dest_id, std::uint8_t dest_entry) {
        uint8_t n = OutgoingConnections();
        this->dest_ids[n] = dest_id;
        this->dest_entry[n] = dest_entry;
        n++;
        setCounter(n);
    }
    static constexpr std::array<void (*)(uint8_t &), 9> OperationTable = {
        [](uint8_t &in) { BitWise::set_bit<LOGIC_GATE_OUTPUT_BIT>(in, BitWise::read_bit<LOGIC_GATE_A_BIT>(in) && BitWise::read_bit<LOGIC_GATE_B_BIT>(in)); }, // 0: And
        [](uint8_t &in) { BitWise::set_bit<LOGIC_GATE_OUTPUT_BIT>(in, BitWise::read_bit<LOGIC_GATE_A_BIT>(in) || BitWise::read_bit<LOGIC_GATE_B_BIT>(in)); }, // 1: Or
        [](uint8_t &in) { BitWise::set_bit<LOGIC_GATE_OUTPUT_BIT>(in, !BitWise::read_bit<LOGIC_GATE_A_BIT>(in)); },                                           // 2: Not
        [](uint8_t &in) { BitWise::set_bit<LOGIC_GATE_OUTPUT_BIT>(in, BitWise::read_bit<LOGIC_GATE_A_BIT>(in) ^ BitWise::read_bit<LOGIC_GATE_B_BIT>(in)); },  // 3: Xor
        [](uint8_t &in) { BitWise::turn_on<LOGIC_GATE_OUTPUT_BIT>(in); },                                                                                     // 4: ON_IN
        [](uint8_t &in) { BitWise::flip_bit<LOGIC_GATE_OUTPUT_BIT>(in); },                                                                                    // 5: Button
        [](uint8_t &) {},                                                                                                                                     // 6: In
        [](uint8_t &) {},                                                                                                                                     // 7: Out
        [](uint8_t &) { throw ::std::out_of_range("UNDEFINED COMPONENT"); }                                                                                   // 8: undefined
    };
    inline void evaluate() {
        OperationTable[static_cast<std::size_t>(type)](in);
    }
    bool getOutput() {
        return BitWise::read_bit<LOGIC_GATE_OUTPUT_BIT>(in);
    }
    template <class F>
        requires std::invocable<F &, std::uint64_t, std::uint8_t>
    void forEachOutgoingConnection(F &&f) const {
        const auto count = OutgoingConnections();

        for (std::size_t i = 0; i < count; ++i) {
            f(dest_ids[i], dest_entry[i]);
        }
    }
    void setInput(std::uint8_t entry, bool value) {
        const auto mask = static_cast<std::uint8_t>(1u << entry);
        if (value)
            this->in |= mask;
        else
            this->in &= static_cast<std::uint8_t>(~mask);
    }
};
struct Circuit {
    void CacheLinesNeeded(long double bytes) {
        std::cout << fmt::format("{} bytes needs {} cache lines\n", bytes, bytes / 64);
    }
    struct Data {
        std::vector<Gate> LogicComponents;
        template <Enums::Components_enum type>
        std::uint64_t addComponent() {
            LogicComponents.emplace_back(type);
            return LogicComponents.size() - 1;
        }
        void connect(std::uint64_t dest_component_id, std::uint8_t dest_component_entry, std::uint64_t src_component_id) {
            LogicComponents.at(src_component_id).connect(dest_component_id, dest_component_entry);
        }
        std::size_t inputsInCircuit() {
            auto ins = 0;
            for (auto &component : LogicComponents) {
                if (component.type == Enums::Components_enum::In) ins++;
            }
            return ins;
        }
        std::size_t outputsInCircuit() {
            auto outs = 0;
            for (auto &component : LogicComponents) {
                if (component.type == Enums::Components_enum::Out) outs++;
            }
            return outs;
        }
    };
    Data data;
    void stepSimulation() {
        for (auto &component : data.LogicComponents) {
            component.evaluate();
            const bool output = component.getOutput();
            component.forEachOutgoingConnection([&](auto destination_id, auto input_entry) {
                Gate &dest = data.LogicComponents[destination_id];
                if (input_entry >= dest.numberOfEntries()) throw std::out_of_range("Invalid entry id");
                dest.setInput(input_entry, output);
            });
        }
    }
};
struct Position {
    std::uint32_t x = 0;
    std::uint32_t y = 0;
    Position(uint16_t x, uint16_t y) : x(x), y(y) {}
};
struct Camera {
    Position pos;
    Camera(Position pos) : pos(pos) {}
    void moveRight() {
        pos.x++;
    }
    void moveLeft() {
        pos.x--;
    }
    void moveUp() {
        pos.y--;
    }
    void moveDown() {
        pos.y++;
    }
};
struct GUI {
    SDL_Window *window = NULL;
    SDL_Renderer *render = NULL;
    enum Texture_Ids : std::uint8_t {
        AND_ID = 0,
        OR_ID,
        XOR_ID,
        NOT_ID,
        ONIN_ID,
        IN_ID,
        OUT_ID,
        BUTTON_ID,
        AMOUNT_OF_IDS
    };
    std::array<SDL_Texture *, AMOUNT_OF_IDS> textures;
    struct GateData {
        Gate *gate = nullptr;
        Position pos = {0, 0};
        GateData(Gate *gate, Position pos) : gate(gate), pos(pos) {}
    };

    std::vector<GateData> gates_to_render;
    std::vector<std::uint64_t> empty_ids;
    std::uint64_t loadCircuitComponentInScreen(Circuit &circuit, std::uint64_t id, Position pos) {
        gates_to_render.emplace_back(&circuit.data.LogicComponents[id], pos);
        return gates_to_render.size() - 1;
    }
    std::uint64_t loadCircuitComponentInScreen(Circuit &circuit, std::uint64_t id, std::uint32_t x, std::uint32_t y) {
        gates_to_render.emplace_back(&circuit.data.LogicComponents[id], Position(x, y));
        return gates_to_render.size() - 1;
    }
    std::uint8_t loadTexture(SDL_Texture *dest, const char *file) {
        dest = IMG_LoadTexture(render, file);
        return (dest) ? 0 : 1;
    }
    GUI() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) exitError(SDL_GetError());
        window = SDL_CreateWindow("Logic Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_FULLSCREEN, SDL_WINDOW_FULLSCREEN, SDL_WINDOW_FULLSCREEN_DESKTOP);
        if (!window) exitError(SDL_GetError());
        render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!render) exitError(SDL_GetError());
        if (loadTexture(textures[AND_ID], "AND-Photoroom.png") != 0) exitError(IMG_GetError());
        if (loadTexture(textures[XOR_ID], "XOR-Photoroom.png") != 0) exitError(IMG_GetError());
        if (loadTexture(textures[OR_ID], "OR-Photoroom.png") != 0) exitError(IMG_GetError());
        if (loadTexture(textures[NOT_ID], "NOT-Photoroom.png") != 0) exitError(IMG_GetError());
    }
    ~GUI() {
        for (SDL_Texture *texture : textures) {
            SDL_DestroyTexture(texture);
        }
        SDL_DestroyRenderer(render);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    void clear() {
    }
};
bool SIMULATING_MODE = false;
Circuit circuit;
void circuitEngineThread() {
    // GUI writes the circuit here
    while (SIMULATING_MODE) {
        circuit.stepSimulation();
    }
}
int main() {
    auto andA = circuit.data.addComponent<Enums::Components_enum::And>();
    auto andB = circuit.data.addComponent<Enums::Components_enum::And>();
    auto orA = circuit.data.addComponent<Enums::Components_enum::Or>();
    auto notA = circuit.data.addComponent<Enums::Components_enum::Not>();
    auto notB = circuit.data.addComponent<Enums::Components_enum::Not>();
    auto inA = circuit.data.addComponent<Enums::Components_enum::ON_IN>();
    auto inB = circuit.data.addComponent<Enums::Components_enum::ON_IN>();
    auto out = circuit.data.addComponent<Enums::Components_enum::Out>();
    circuit.data.connect(notA, 0, inA);
    circuit.data.connect(andA, 0, notA);
    circuit.data.connect(andA, 1, inB); // AndA = notA and B
    circuit.data.connect(notB, 0, inB);
    circuit.data.connect(andB, 0, notB);
    circuit.data.connect(andB, 1, inA); // AndB = notB and A
    circuit.data.connect(orA, 0, andA);
    circuit.data.connect(orA, 1, andB);
    circuit.data.connect(out, 0, orA);
    std::jthread simulationEngine(circuitEngineThread);
    GUI gui;
    gui.loadCircuitComponentInScreen(circuit, andA, 0, 0);
    return 0;
}

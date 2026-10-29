#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL_error.h>
#include <SDL_render.h>
#include <SDL_video.h>

#include "../include/position.hpp"
#include "gates.hpp"
#include "circuit.hpp"

#include <array>
#include <cstdint>
#include <vector>

struct Camera {
    Position pos;

    Camera(Position pos);

    void moveRight();
    void moveLeft();
    void moveUp();
    void moveDown();
};

struct GUI {
    SDL_Window* window = NULL;
    SDL_Renderer* render = NULL;

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

    std::array<SDL_Texture*, AMOUNT_OF_IDS> textures;

    struct GateData {
        Gate* gate = nullptr;
        Position pos = {0, 0};

        GateData(Gate* gate, Position pos)
            : gate(gate), pos(pos) {}
    };

    std::vector<GateData> gates_to_render;
    std::vector<std::uint64_t> empty_ids;

    std::uint64_t loadCircuitComponentInScreen(
        Circuit& circuit,
        std::uint64_t id,
        Position pos
    );

    std::uint64_t loadCircuitComponentInScreen(
        Circuit& circuit,
        std::uint64_t id,
        std::uint32_t x,
        std::uint32_t y
    );

    std::uint8_t loadTexture(SDL_Texture* dest, const char* file);

    GUI();

    ~GUI();

    void clear();
};

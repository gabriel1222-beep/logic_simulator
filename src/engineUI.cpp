
#include "engineUI.hpp"

#include "helper.hpp"
// --------------------------------------------------
// Camera
// --------------------------------------------------

Camera::Camera(Position pos)
    : pos(pos) {}

void Camera::moveRight() {
    // Original implementation not provided.
}

void Camera::moveLeft() {
    // Original implementation not provided.
}

void Camera::moveUp() {
    // Original implementation not provided.
}

void Camera::moveDown() {
    // Original implementation not provided.
}

// --------------------------------------------------
// GUI
// --------------------------------------------------

std::uint64_t GUI::loadCircuitComponentInScreen(
    Circuit& circuit,
    std::uint64_t id,
    Position pos
) {
    gates_to_render.emplace_back(
        &circuit.data.LogicComponents[id],
        pos
    );

    return gates_to_render.size() - 1;
}

std::uint64_t GUI::loadCircuitComponentInScreen(
    Circuit& circuit,
    std::uint64_t id,
    std::uint32_t x,
    std::uint32_t y
) {
    gates_to_render.emplace_back(
        &circuit.data.LogicComponents[id],
        Position(x, y)
    );

    return gates_to_render.size() - 1;
}

std::uint8_t GUI::loadTexture(
    SDL_Texture* dest,
    const char* file
) {
    dest = IMG_LoadTexture(render, file);

    return (dest) ? 0 : 1;
}

GUI::GUI() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        exitError(SDL_GetError());

    window = SDL_CreateWindow(
        "Logic Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOW_FULLSCREEN,
        SDL_WINDOW_FULLSCREEN,
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );

    if (!window)
        exitError(SDL_GetError());

    render = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!render)
        exitError(SDL_GetError());

    if (loadTexture(textures[AND_ID], "AND-Photoroom.png") != 0)
        exitError(IMG_GetError());

    if (loadTexture(textures[XOR_ID], "XOR-Photoroom.png") != 0)
        exitError(IMG_GetError());

    if (loadTexture(textures[OR_ID], "OR-Photoroom.png") != 0)
        exitError(IMG_GetError());

    if (loadTexture(textures[NOT_ID], "NOT-Photoroom.png") != 0)
        exitError(IMG_GetError());
}

GUI::~GUI() {
    for (SDL_Texture* texture : textures) {
        SDL_DestroyTexture(texture);
    }

    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void GUI::clear() {
}

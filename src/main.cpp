#include "circuit.hpp"
#include "gates.hpp"
#include "helper.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fmt/base.h>
#include <fmt/format.h>
#include <functional>
#include <print>
#include <ratio>
#include <string>
#include <sys/types.h>
#include <thread>
enum Default_color {
    // --- Basic Colors ---
    Black,
    White,
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta,

    // --- Secondary & Popular Colors ---
    Orange,
    Purple,
    Gray,
    Brown,
    Pink,
    Lime,
    Teal,
    Maroon,
    Navy,
    Olive,

    // --- Light / Dark Variations ---
    DarkGray,
    LightGray,
    Crimson,
    SkyBlue,

    // --- Special / UI Colors ---
    Gold,
    Silver,
    Transparent,

    Colors // Automatically evaluates to 25
};

constexpr static const std::array<std::array<uint8_t, 4>, Default_color::Colors> color_table = {{
    // --- Basic Colors ---
    {0, 0, 0, 255},       // Black
    {255, 255, 255, 255}, // White
    {255, 0, 0, 255},     // Red
    {0, 128, 0, 255},     // Green (Standard Darker Green)
    {0, 0, 255, 255},     // Blue
    {255, 255, 0, 255},   // Yellow
    {0, 255, 255, 255},   // Cyan
    {255, 0, 255, 255},   // Magenta

    // --- Secondary & Popular Colors ---
    {255, 165, 0, 255},   // Orange
    {128, 0, 128, 255},   // Purple
    {128, 128, 128, 255}, // Gray
    {165, 42, 42, 255},   // Brown
    {255, 192, 203, 255}, // Pink
    {0, 255, 0, 255},     // Lime (Bright Vibrant Green)
    {0, 128, 128, 255},   // Teal
    {128, 0, 0, 255},     // Maroon
    {0, 0, 128, 255},     // Navy
    {128, 128, 0, 255},   // Olive

    // --- Light / Dark Variations ---
    {64, 64, 64, 255},    // DarkGray
    {192, 192, 192, 255}, // LightGray
    {220, 20, 60, 255},   // Crimson
    {135, 206, 235, 255}, // SkyBlue

    // --- Special / UI Colors ---
    {255, 215, 0, 255},   // Gold
    {245, 245, 245, 255}, // Silver
    {0, 0, 0, 0}          // Transparent (Alpha = 0)
}};

struct GUI {
  private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    std::array<SDL_Texture *, 4> textures;
    int loadTexture(const char *path, SDL_Texture **texture) {
        *texture = IMG_LoadTexture(renderer, path);
        if (!*texture) {
            return 1;
        } else {
            return 0;
        }
    }
    float mod(float value) {
        return (value > 0) ? value : -value;
    }

  public:
    GUI() {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            exitError(SDL_GetError());
        }
        window = SDL_CreateWindow(
            "Logism++",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            0, 0, // Width and Height are ignored when using FULLSCREEN_DESKTOP
            SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);

        if (!window) {
            exitError(SDL_GetError());
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            exitError(SDL_GetError());
        }
        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            exitError(IMG_GetError());
        }
        if (loadTexture("AND-Photoroom.png", &textures[0]) != 0) {
            exitError(IMG_GetError());
        }
        if (loadTexture("NOT-Photoroom.png", &textures[1]) != 0) {
            exitError(IMG_GetError());
        }
        if (loadTexture("OR-Photoroom.png", &textures[2]) != 0) {
            exitError(IMG_GetError());
        }
        if (loadTexture("XOR-Photoroom.png", &textures[3]) != 0) {
            exitError(IMG_GetError());
        }
    }
    void drawImg(int i,int x, int y) {
        int imageW, imageH;
        SDL_QueryTexture(textures[i], nullptr, nullptr, &imageW, &imageH);
        SDL_Rect destination{x, y, imageW, imageH};
        SDL_RenderCopy(renderer, textures[i], nullptr, &destination);
    }
    void setDrawColor(Default_color color) {
        SDL_SetRenderDrawColor(renderer, color_table[color][0], color_table[color][1], color_table[color][2], color_table[color][3]);
    }
    void drawLine(int x1, int y1, int x2, int y2) {
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
    void clearWindow() {
        SDL_RenderClear(renderer);
    }
    void preset() {
        SDL_RenderPresent(renderer);
    }
    void renderLoop(uint8_t fps, std::function<void(bool *running)> func) {
        auto last_time = std::chrono::steady_clock::now();
        bool running = true;

        const float target_dt = 1.0f / fps;
        float sleeping_time = target_dt;
        float accumulated_error = 0.0f; // Stores historical drift

        while (running) {
            auto frame_start = std::chrono::steady_clock::now();

            if (func) {
                func(&running);
            }

            auto frame_end = std::chrono::steady_clock::now();

            // 1. Calculate how long the CPU spent inside your frame logic
            std::chrono::duration<float> work_elapsed = frame_end - frame_start;
            float work_time = work_elapsed.count();

            // 2. Base sleep time is what remains of our target window
            float base_sleep = target_dt - work_time;

            // 3. Measure total overall frame duration (including previous sleep)
            std::chrono::duration<float> total_elapsed = frame_end - last_time;
            last_time = frame_end;
            float actual_dt = total_elapsed.count();

            // 4. Calculate drift error (positive if too fast, negative if lagging)
            float current_error = target_dt - actual_dt;
            accumulated_error += current_error;

            // 5. Update sleeping time using a dampening factor (0.1f) on historical error
            sleeping_time = base_sleep + (accumulated_error * 0.1f);

            // 6. MANDATORY CRASH PROTECTION: Clamp sleep limits
            if (sleeping_time < 0.0f) {
                sleeping_time = 0.0f;
            }
            if (sleeping_time > target_dt) {
                sleeping_time = target_dt;
            }

            // 7. Execute the precise sleep
            std::this_thread::sleep_for(std::chrono::duration<float>(sleeping_time));

            // Output diagnostics using real timing values
            if (actual_dt > 0.0f) {
                fmt::println("Fps = {}", 1.0f / actual_dt);
                fmt::println("Sleeping time = {}", sleeping_time);
            }
        }
    }
    ~GUI() {
        SDL_DestroyWindowSurface(window);
        SDL_DestroyRenderer(renderer);
        for (SDL_Texture *texture : textures) {
            SDL_DestroyTexture(texture);
        }
    }
};
bool SIMULATING_MODE = false;
int main() {
    Circuit circuit;
    auto notA = circuit.data.addComponent<Enums::Components_enum::Not>();
    auto inB = circuit.data.addComponent<Enums::Components_enum::ON_IN>();
    auto andA = circuit.data.addComponent<Enums::Components_enum::And>();
    circuit.data.connect(andA, 1, inB); // AndA = notA and B
    circuit.data.connect(andA, 0, notA);
    auto andB = circuit.data.addComponent<Enums::Components_enum::And>();
    auto out = circuit.data.addComponent<Enums::Components_enum::Out>();
    auto orA = circuit.data.addComponent<Enums::Components_enum::Or>();
    circuit.data.connect(out, 0, orA);
    circuit.data.connect(orA, 0, andA);
    circuit.data.connect(orA, 1, andB);
    auto inA = circuit.data.addComponent<Enums::Components_enum::ON_IN>();
    circuit.data.connect(andB, 1, inA); // AndB = notB and A
    circuit.data.connect(notA, 0, inA);
    auto notB = circuit.data.addComponent<Enums::Components_enum::Not>();
    circuit.data.connect(andB, 0, notB);
    circuit.data.connect(notB, 0, inB);
    GUI gui;
    SDL_Event event;
    gui.renderLoop(30, [&](auto *running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                *running = false;
            }
        }
        gui.setDrawColor(Default_color::White);
        gui.clearWindow();
        gui.setDrawColor(Default_color::White);
        gui.drawImg(0, 300, 300);
        gui.preset();
    });

    return 0;
}

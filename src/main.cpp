#include "SDL_error.h"
#include "SDL_render.h"
#include "SDL_timer.h"
#include "mover/mover.hpp"
#include <SDL.h>
#include <array>
#include <cstdint>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

struct Player {
  Player() : mover(Mover(rand() % 5 + 10)) {}
  Mover mover;

  void update(double delta_time) { mover.update(delta_time); }

  void render(SDL_Renderer *renderer) const {
    SDL_Rect mover_rect{
        .x = static_cast<int>(mover.position().x),
        .y = static_cast<int>(mover.position().y),
        .w = static_cast<int>(mover.mass() * 5.0f),
        .h = static_cast<int>(mover.mass() * 5.0f),
    };

    SDL_SetRenderDrawColor(renderer, 200, 75, 23, 100);
    SDL_RenderFillRect(renderer, &mover_rect);
  }
};

struct Liquid {
  Liquid(const Vector2f &position, const Vector2f &dim)
      : position(position), dim(dim) {}

  Vector2f position;
  Vector2f dim;

  SDL_Rect rect() const {
    return SDL_Rect{
        .x = static_cast<int>(position.x),
        .y = static_cast<int>(position.y),
        .w = static_cast<int>(dim.x),
        .h = static_cast<int>(dim.y),
    };
  }

  void update(double delta_time) {}

  void render(SDL_Renderer *renderer) const {
    SDL_SetRenderDrawColor(renderer, 0, 0, 175, 50);

    SDL_Rect liquid_rect = this->rect();

    SDL_RenderFillRect(renderer, &liquid_rect);
    SDL_SetRenderDrawColor(renderer, 75, 75, 175, 255);
    SDL_RenderDrawRect(renderer, &liquid_rect);
  }

  bool contains(const Mover &mover) const {
    SDL_Rect liquid_rect = this->rect();
    SDL_Rect mover_rect{
        .x = static_cast<int>(mover.position().x),
        .y = static_cast<int>(mover.position().y),
        .w = static_cast<int>(mover.mass() * 5.0f),
        .h = static_cast<int>(mover.mass() * 5.0f),
    };

    return SDL_HasIntersection(&liquid_rect, &mover_rect);
  }
};

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
bool isPlaying = true;
std::array<Player, 3> players;
Liquid liquid(Vector2f(0, 400), Vector2f(800, 200));

// Global time tracking for delta time
double delta_time = 0.0;
uint64_t LAST = 0;
const double desired_frame_time = 1.0 / 60.0; // 60 FPS

int init();
void handleEvents();
void update(double delta_time);
void render();
void cleanup();

#ifdef __EMSCRIPTEN__
// Emscripten main loop wrapper
void emscripten_game_loop(void) {
  uint64_t NOW = SDL_GetPerformanceCounter();
  delta_time = (NOW - LAST) / static_cast<double>(SDL_GetPerformanceFrequency());
  LAST = NOW;

  handleEvents();
  update(delta_time);
  render();

  // Gracefully exit if the game is stopped
  if (!isPlaying) {
    cleanup();
    emscripten_cancel_main_loop(); // Stop the main loop
  }
}
#endif

int main(int argc, char *argv[]) {
  if (init() < 0) {
    std::cerr << "Failed to initialize the application, quitting." << std::endl;
    return -1;
  }

  LAST = SDL_GetPerformanceCounter(); // Initialize LAST time

#ifdef __EMSCRIPTEN__
  // Use Emscripten main loop
  emscripten_set_main_loop(emscripten_game_loop, 0, 1);
#else
  // Native main loop
  while (isPlaying) {
    uint64_t NOW = SDL_GetPerformanceCounter();
    delta_time = (NOW - LAST) / static_cast<double>(SDL_GetPerformanceFrequency());
    LAST = NOW;

    handleEvents();
    update(delta_time);
    render();

    uint64_t frame_end_time = SDL_GetPerformanceCounter();
    double frame_time = (frame_end_time - NOW) /
                        static_cast<double>(SDL_GetPerformanceFrequency());

    if (frame_time < desired_frame_time) {
      double delay_time = (desired_frame_time - frame_time) * 1000.0;
      SDL_Delay(static_cast<Uint32>(delay_time));
    }
  }

  cleanup();
#endif

  return 0;
}

int init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to initialize the SDL2 Library; Error = "
              << SDL_GetError() << std::endl;
    return -1;
  }

  window =
      SDL_CreateWindow("wasm-ball-z", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

  if (window == nullptr) {
    std::cerr << "Failed to create a window; Error = " << SDL_GetError()
              << std::endl;
    return -1;
  }

  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == nullptr) {
    std::cerr << "Failed to create a renderer; Error = " << SDL_GetError()
              << std::endl;
    return -1;
  }

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  std::cout << "Successfully initialized the application." << std::endl;

  return 0;
}

void handleEvents() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_QUIT:
      isPlaying = false;
      break;
    case SDL_KEYDOWN:
      switch (e.key.keysym.sym) {
      case SDLK_ESCAPE:
        isPlaying = false;
        break;
      }
      break;
    }
  }
}

void update(double delta_time) {
  Vector2f gravity(0.0f, 1000.f);
  Vector2f wind(300.0f, 0.f);

  for (Player &player : players) {
    player.mover.add_force(wind);
    player.mover.add_force(gravity.mul(player.mover.mass()));

    if (player.mover.position().y >= 600 - (player.mover.mass() * 5.0f)) {
      float coefficient_of_friction = 0.1f;
      float gravity_magnitude = gravity.y;
      float normal_force = player.mover.mass() * gravity_magnitude;

      float friction_magnitude = coefficient_of_friction * normal_force;

      Vector2f friction = player.mover.velocity().normalized().mul(-1.0f).mul(
          friction_magnitude);

      player.mover.add_force(friction);
    }

    if (liquid.contains(player.mover)) {
      float drag_coefficient = 0.1f;
      float drag_magnitude = drag_coefficient * player.mover.velocity().mag() *
                             player.mover.velocity().mag();

      Vector2f drag_force =
          player.mover.velocity().normalized().mul(-1.0f).mul(drag_magnitude);

      player.mover.add_force(drag_force);
    }

    player.update(delta_time);
  }
}

void render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  for (const Player &player : players) {
    player.render(renderer);
  }

  liquid.render(renderer);

  SDL_RenderPresent(renderer);
}

void cleanup() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  std::cout << "Successfully cleaned up the application." << std::endl;
}

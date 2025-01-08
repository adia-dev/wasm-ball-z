#pragma once

#include "utils/r.hpp"
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace wbz {
class TextRenderer {
public:
  static TextRenderer &instance() {
    static TextRenderer instance;
    return instance;
  }

  void init() {
    if (TTF_Init() == -1) {
      throw std::runtime_error("Failed to initialize SDL_ttf");
    }
    load_fonts();
  }

  void render_text(SDL_Renderer *renderer, const std::string &text, int x,
                   int y, SDL_Color color, int size = 16) {
    TTF_Font *font = get_font(size);
    if (!font)
      return;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface)
      return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
      SDL_Rect dest = {x, y, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, nullptr, &dest);
      SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
  }

  void cleanup() {
    _fonts.clear();
    TTF_Quit();
  }

private:
  TextRenderer() = default;
  std::unordered_map<int, std::shared_ptr<TTF_Font>> _fonts;

  void load_fonts() {
    std::vector<int> sizes = {12, 16, 24, 32};

    for (int size : sizes) {
      TTF_Font *font =
          TTF_OpenFont((utils::R::fonts() + "seguiemj.ttf").c_str(), size);
      if (font) {
        _fonts[size] = std::shared_ptr<TTF_Font>(font, TTF_CloseFont);
      }
    }
  }

  TTF_Font *get_font(int size) {
    auto it = _fonts.find(size);
    return it != _fonts.end() ? it->second.get() : nullptr;
  }
};
} // namespace wbz

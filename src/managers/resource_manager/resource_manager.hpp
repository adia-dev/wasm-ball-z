#include <SDL2/SDL_image.h>
#include <SDL_render.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utils/r.hpp>

namespace fs = std::filesystem;

namespace wbz {
namespace managers {

class ResourceManager {
public:
  enum class PathPolicy {
    RELATIVE,
    ABSOLUTE,
  };

  static ResourceManager &instance() {
    static ResourceManager instance;
    return instance;
  }

  static std::shared_ptr<SDL_Texture>
  get_texture(SDL_Renderer *renderer, const fs::path &file_path,
              PathPolicy policy = PathPolicy::RELATIVE) {

    auto &resource_manager = instance();

    fs::path adjusted_file_path =
        policy == PathPolicy::ABSOLUTE
            ? file_path
            : fs::path(utils::R::textures() + "/" + file_path.string());

    auto found_texture =
        resource_manager._textures.find(adjusted_file_path.string());
    if (found_texture != resource_manager._textures.end()) {
      /* std::cout << "Texture found in cache: " << adjusted_file_path << "\n";
       */
      return found_texture->second;
    }

    if (!fs::exists(adjusted_file_path)) {
      std::cerr << "File not found at: " << adjusted_file_path
                << " (does not exist)\n";
      throw std::runtime_error("Failed to load texture: File not found");
    }

    SDL_Surface *surface = IMG_Load(adjusted_file_path.c_str());
    if (surface == nullptr) {
      std::cerr << "Failed to load image at path: " << adjusted_file_path
                << "; Error: " << IMG_GetError() << "\n";
      throw std::runtime_error("Failed to load image: Error using IMG_Load()");
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
      std::cerr << "Failed to create texture from surface; Error: "
                << SDL_GetError() << "\n";
      SDL_FreeSurface(surface);
      throw std::runtime_error("Failed to create texture from surface");
    }

    SDL_FreeSurface(surface);

    resource_manager._textures.emplace(
        adjusted_file_path.string(),
        std::shared_ptr<SDL_Texture>(texture, SDL_DestroyTexture));

    return resource_manager._textures[adjusted_file_path.string()];
  }

private:
  ResourceManager() = default;

  ~ResourceManager() {
    std::for_each(_textures.begin(), _textures.end(), [](auto &entry) {
      SDL_DestroyTexture(entry.second.get());
      std::cout << "Successfully destroyed texture: " << entry.first << "\n";
    });
    _textures.clear();
  }

  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;

  std::unordered_map<std::string, std::shared_ptr<SDL_Texture>> _textures;
};

} // namespace managers
} // namespace wbz

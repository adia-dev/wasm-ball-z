#pragma once

#include <string>

namespace wbz {
namespace utils {
struct R {
  static const std::string &textures() {
    static std::string path = std::string(RESOURCE_DIR) + "/textures";
    return path;
  }
};
} // namespace utils
} // namespace wbz

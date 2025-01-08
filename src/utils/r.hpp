#pragma once

#include <string>

namespace wbz {
namespace utils {
struct R {
  static const std::string &textures() {
    static std::string path = std::string(RESOURCE_DIR) + "/textures/";
    return path;
  }

  static const std::string &fonts() {
    static std::string path = std::string(RESOURCE_DIR) + "/fonts/";
    return path;
  }

  static const std::string &animations() {
    static std::string path = std::string(RESOURCE_DIR) + "/animations/";
    return path;
  }
};
} // namespace utils
} // namespace wbz

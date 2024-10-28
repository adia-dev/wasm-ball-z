#include <string>
namespace wbz {
namespace utils {
struct R {
  static std::string textures() {
    static std::string path = std::string(RESOURCE_DIR) + "/textures";
    return path;
  }
};
} // namespace utils
} // namespace wbz

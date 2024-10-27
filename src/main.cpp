#include "application/application.hpp"

int main(int argc, char *argv[]) {
  try {
    wbz::Application::run();
  } catch (...) {
    std::cerr << "Application closed with an exception\n";
  }
  return 0;
}

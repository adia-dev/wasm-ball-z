#include <iostream>

#include "application/application.hpp"

int main(int argc, char *argv[]) {
  try {
    wbz::Application::run();
  } catch (...) {
    wbz::Application::shutdown();
    std::cerr << "Application shutdown with an exception\n";
  }
  return 0;
}

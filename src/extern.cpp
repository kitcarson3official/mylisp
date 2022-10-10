#include "linenoise.hpp"
#include <string>
#include <iostream>

namespace ml {
std::string readline(std::string input) {
  std::string ret;
  std::cout << input;
  std::cin >> ret;
  return ret;
}
} // namespace ml

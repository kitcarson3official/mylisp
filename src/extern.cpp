#include "linenoise.hpp"
#include <string>

namespace ml {
std::string readline(std::string input) {
  std::string ret;
  linenoise::Readline(input.c_str(), ret);
  return ret;
}
} // namespace ml

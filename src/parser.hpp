#pragma once
#include "types.hpp"
#include "debug.hpp"
#include <stack>

namespace ml {
class Parser {
public:
  Parser();
  shared_ptr<Object> parse(std::string input);
#ifdef DEBUG_Parser_debug
  void debug();
#endif
private:
  shared_ptr<Object> parse_form();
  shared_ptr<Object> parse_atom();
  shared_ptr<List>   parse_list();
  shared_ptr<Vec>    parse_vec();
  shared_ptr<Dict>   parse_dict();
  void tokenize();
  std::string_view peak();
  std::string_view next();
  void reset();

  std::string _input;
  int token_parse_index;
  std::vector<std::string_view> tokens;
  enum BRACKETS {
    CURVE,
    SQUARE,
    GRAPH,
  };
  std::stack<BRACKETS> brackets;
};
} // namespace ml

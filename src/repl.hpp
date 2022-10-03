#pragma once
#include "env.hpp"
#include "types.hpp"

namespace ml {
shared_ptr<Object> READ(std::string input);
shared_ptr<Object> EVAL(shared_ptr<Object> input, shared_ptr<Environment> env);
std::string PRINT(shared_ptr<Object> input);
std::string rep(std::string input, shared_ptr<Environment> rep_env);
shared_ptr<Object> quasiquote(shared_ptr<Object> ast);
bool is_macro_call(shared_ptr<Object> ast, shared_ptr<Environment> env);
shared_ptr<Object> macroexpand(shared_ptr<Object> ast,
                               shared_ptr<Environment> env);
extern const std::vector<std::string> keywords;

class Runtime {
public:
  Runtime();
  int repl();
  static shared_ptr<Object> message_signal;
  static shared_ptr<Object> unhandled_exc;
  static shared_ptr<Exception> ret_exception(std::string message);
  shared_ptr<Environment> env();
  bool running;

private:
  shared_ptr<Environment> core_env;
};

} // namespace ml

#pragma once
#include "types.hpp"
#include <unordered_map>

namespace ml {

class Environment : public Object,
                    public std::enable_shared_from_this<Environment> {
public:
  Environment(shared_ptr<Environment> outer = to<Environment, Nil>(nil()));
  void set(shared_ptr<Object> key, shared_ptr<Object> value);
  shared_ptr<Environment> find(shared_ptr<Symbol> key);
  shared_ptr<Object> get(shared_ptr<Symbol> key);
  std::string get_key(shared_ptr<Object> obj);

private:
  std::unordered_map<std::string, shared_ptr<Object>> map;
  shared_ptr<Environment> _outer;
};

shared_ptr<Environment> to_environment(shared_ptr<Object> o);
} // namespace ml

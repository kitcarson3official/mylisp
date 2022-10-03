#include "env.hpp"
#include "printer.hpp"
#include "repl.hpp"
#include <iostream>
using std::cout, std::endl;

namespace ml {
Environment::Environment(shared_ptr<Environment> outer) : Object(ENVIRONMENT) {
  _outer = outer;
}

void Environment::set(shared_ptr<Object> key, shared_ptr<Object> value) {
  switch (key->type) {
  case STRING:
    map.insert_or_assign(to_str(key)->value(), value);
    break;
  case SYMBOL:
    map.insert_or_assign(to_symbol(key)->value(), value);
    break;
  default:
    cout << "key type not valid (only string or keyword may be a key)" << endl;
    exit(1);
  }
}

shared_ptr<Environment> Environment::find(shared_ptr<Symbol> key) {
  std::string key_string = key->value();
  if (map.contains(key_string)) {
    return shared_from_this();
  } else {
  }
  if (_outer->type == ENVIRONMENT and _outer->find(key)->type == ENVIRONMENT) {
    return _outer->find(key);
  } else {
    return to<Environment, Nil>(nil());
  }
}

shared_ptr<Object> Environment::get(shared_ptr<Symbol> key) {
  shared_ptr<Environment> fe = find(key);
  if (fe->type == ENVIRONMENT) {
    std::string key_string;
    switch (key->type) {
    case STRING:
      key_string = to_str(key)->value();
      break;
    case SYMBOL:
      key_string = to_symbol(key)->value();
      break;
    default:
      cout << "key type not valid (only string or keyword may be a key)"
           << endl;
      exit(1);
      to_environment(nil());
    }
    auto ret = fe->map.at(key_string);
    return ret;
  } else {
    shared_ptr<Exception> exc =
        exception("Symbol not found exception.\n" + to_symbol(key)->value());
    Runtime::unhandled_exc = exc;
    return nil();
  }
}

shared_ptr<Environment> to_environment(shared_ptr<Object> o) {
  return std::static_pointer_cast<Environment>(o);
}
} // namespace ml

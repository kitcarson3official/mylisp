#include "types.hpp"
#include "env.hpp"
#include "printer.hpp"
#include "repl.hpp"
#include <iostream>
#ifdef DEBUG_Types_info
#include "printer.hpp"
#endif
using std::cout, std::endl, std::make_shared;

namespace ml {

Object::Object(OBJECT_TYPE o_type) { type = o_type; }

// ROOT TYPE
Root::Root() : Object(ROOT) {}

// ATOM TYPE

Atom::Atom(shared_ptr<Object> o) : Object(ATOM) { this->content = o; }

void Atom::set(shared_ptr<Object> o) { this->content = o; }
shared_ptr<Object> Atom::value() { return this->content; }
OBJECT_TYPE Atom::value_type() { return this->content->type; }

// EXCEPTION TYPE

Exception::Exception(std::string message) : Object(EXCEPTION) {
  this->message = message;
}

std::string Exception::value() const { return message; }

// INNER TYPE

Signal::Signal(INNER_SIGNALS value) : Object(SIGNAL) { _value = value; }

// NILL

Nil::Nil() : Object(NIL) {}

// SYMBOL

Symbol::Symbol(std::string value) : Object(SYMBOL) { this->_value = value; }

const std::string &Symbol::value() const { return _value; }

// BOOL

Bool::Bool(bool value) : Object(BOOL) { _value = value; }

const bool Bool::value() const { return _value; }

const bool Bool::operator==(const shared_ptr<Bool> other) {
  return value() == other->value();
}

// KEYWORD

Keyword::Keyword(std::string value) : Object(KEYWORD) { this->_value = value; }

const std::string &Keyword::value() const { return _value; }

// NUMBER

Number::Number(double value) : Object(NUMBER) { this->_value = value; }

const double Number::value() const { return _value; }

const bool Number::operator==(const shared_ptr<Number> other) {
  return value() == other->value();
}
// STRING

Str::Str(std::string value) : Object(STRING) { this->_value = value; }

const std::string &Str::value() const { return _value; }

const bool Str::operator==(const shared_ptr<Str> other) {
  return value() == other->value();
}

// LIST

List::List() : Object(LIST), meta(nil()) {}

void List::append(shared_ptr<Object> obj) { elements.push_back(obj); }

shared_ptr<Object> List::operator[](unsigned int index) {
  if (index < elements.size())
    return elements[index];
  else {
    cout << "index out of bounds" << endl;
    return nil();
  }
}

const bool List::operator==(const shared_ptr<List> other) {
  if (elements.size() == other->elements.size()) {
    for (unsigned int i = 0; i < elements.size(); i++) {
      if (elements[i]->type == other->elements[i]->type) {
        switch (elements[i]->type) {
        case BOOL:
          if (not(*(to_bool(elements[i])) == to_bool(other->elements[i]))) {
            return false;
          }
          break;
        case NUMBER:
          if (not(*(to_number(elements[i])) == to_number(other->elements[i]))) {
            return false;
          }
          break;
        case STRING:
          if (not(*(to_str(elements[i])) == to_str(other->elements[i]))) {
            return false;
          }
          break;
        case LIST:
          if (not(*(to_list(elements[i])) == to_list(other->elements[i]))) {
            return false;
          }
          break;
        case VEC:
          if (not(*(to_vec(elements[i])) == to_vec(other->elements[i]))) {
            return false;
          }
          break;
        case DICT:
          if (not(*(to_dict(elements[i])) == to_dict(other->elements[i]))) {
            return false;
          }
          break;
        default:
          cout << "= operation not only supported for bool, number, string, "
                  "list, vec, dict"
               << endl;
          return false;
        }
      } else {
        return false;
      }
    }
  } else
    return false;
  return true;
}

// VECTOR

Vec::Vec() : Object(VEC), meta(nil()) {}

void Vec::append(shared_ptr<Object> obj) { elements.push_back(obj); }

shared_ptr<Object> Vec::operator[](unsigned int index) {
  if (index < elements.size())
    return elements[index];
  else {
    cout << "index out of bounds" << endl;
    return nil();
  }
}

const bool Vec::operator==(const shared_ptr<Vec> other) {
  if (elements.size() == other->elements.size()) {
    for (unsigned int i = 0; i < elements.size(); i++) {
      if (elements[i]->type == other->elements[i]->type) {
        switch (elements[i]->type) {
        case BOOL:
          if (not(*(to_bool(elements[i])) == to_bool(other->elements[i]))) {
            return false;
          }
          break;
        case NUMBER:
          if (not(*(to_number(elements[i])) == to_number(other->elements[i]))) {
            return false;
          }
          break;
        case STRING:
          if (not(*(to_str(elements[i])) == to_str(other->elements[i]))) {
            return false;
          }
          break;
        case LIST:
          if (not(*(to_list(elements[i])) == to_list(other->elements[i]))) {
            return false;
          }
          break;
        case VEC:
          if (not(*(to_vec(elements[i])) == to_vec(other->elements[i]))) {
            return false;
          }
          break;
        case DICT:
          if (not(*(to_dict(elements[i])) == to_dict(other->elements[i]))) {
            return false;
          }
          break;
        default:
          cout << "= operation not only supported for bool, number, string, "
                  "list, vec, dict"
               << endl;
          return false;
        }
      } else {
        return false;
      }
    }
  } else
    return false;
  return true;
}
// DICT

Dict::Dict() : Object(DICT), meta(nil()) {}

void Dict::append(shared_ptr<Object> key, shared_ptr<Object> value) {
  if (key->type == STRING or key->type == KEYWORD)
    map.insert_or_assign(key, value);
  else
    Runtime::unhandled_exc =
        exception("dictionary keys must be string or keywords");
}

shared_ptr<Object> Dict::operator[](shared_ptr<Object> key) {
  if (map.contains(key))
    return map[key];
  else
    return nil();
}

const bool Dict::operator==(const shared_ptr<Dict> other) {
  if (map.size() == other->map.size()) {
    for (auto el : map) {
      auto key = el.first;
      if (not other->map.contains(key))
        return false;
      else {
        if (map[key]->type == other->map[key]->type) {
          switch (map[key]->type) {
          case BOOL:
            if (not(*to_bool(map[key]) == to_bool(other->map[key]))) {
              return false;
            }
            break;
          case NUMBER:
            if (not(*to_number(map[key]) == to_number(map[key]))) {
              return false;
            }
            break;
          case STRING:
            if (not(*to_str(map[key]) == to_str(other->map[key]))) {
              return false;
            }
            break;
          case LIST:
            if (not(*to_list(map[key]) == to_list(other->map[key]))) {
              return false;
            }
            break;
          case VEC:
            if (not(*(to_vec(map[key])) == to_vec(other->map[key]))) {
              return false;
            }
            break;
          case DICT:
            if (not(*(to_dict(map[key])) == to_dict(other->map[key]))) {
              return false;
            }
            break;
          default:
            cout << "= operation not only supported for bool, number, string, "
                    "list, vec, dict"
                 << endl;
            return false;
          }
        } else {
          return false;
        }
      }
    }
  } else
    return false;
  return true;
}

// FUNCTION

Function::Function(std::function<shared_ptr<Object>(shared_ptr<List>)> f,
                   std::string name, std::string help)
    : Object(FUNCTION), meta(nil()) {
  compiled = true;
  this->name = name;
  this->f = f;
  this->arguments = to_list(nil());
  this->expression = to_obj(nil());
}

Function::Function(shared_ptr<List> arguments, shared_ptr<Object> expression,
                   shared_ptr<Environment> env, std::string name,
                   std::string help, bool is_macro)
    : Object(FUNCTION), meta(nil()) {
  /*
   * if the function has been called with list of arguments and list of
   * expressions it must be an interpreted function and not a compiled
   * one.
   * */
  compiled = false;
  this->f = nullptr;
  /*
   * here it is checked if the function has as the last argument a variadic
   * argument. in such case it's position is saved int the variable
   * *last_is_variadic* so to easy assign arguments to the variadic list when
   * the function is called.
   * */
  for (unsigned int i = 0; i < arguments->elements.size(); i++) {
    if (to_symbol(arguments->elements[i])->value() == "&") {
      if (i == arguments->elements.size() - 2) {
        last_is_variadic = i;
      } else {
        cout << "variadic symbol & must precede the last parameter name"
             << endl;
        exit(1);
      }
    }
  }

  if (last_is_variadic >= 0) { // then is a variadic function
    this->arguments = list();
    for (unsigned int i = 0; i < arguments->elements.size(); i++)
      if (i != arguments->elements.size() - 2) // skip the & symbol
        this->arguments->append(
            symbol(to_symbol(arguments->elements[i])->value()));
  } else {
    this->arguments = arguments;
  }
  this->expression = expression;
  this->calling_env = env;
  this->is_macro = is_macro;
}

shared_ptr<Object> Function::call(shared_ptr<List> args) {
  if (compiled)
    return f(args);
  else {
    shared_ptr<Environment> closure = make_shared<Environment>(calling_env);
    if (last_is_variadic >= 0) {
      if (arguments->elements.size() >= last_is_variadic) {
        for (unsigned int i = 0; i < last_is_variadic; i++)
          closure->set(arguments->elements[i], args->elements[i]);
        shared_ptr<List> varargs = list();
        for (unsigned int i = last_is_variadic; i < arguments->elements.size();
             i++)
          varargs->append(arguments->elements[i]);
        closure->set(arguments->elements[last_is_variadic], varargs);
      } else {
        return to_obj(Runtime::ret_exception(
            "Funcion <" + calling_env->get_key(shared_from_this()) +
            ">: wrong number of parameters"));
      }
    } else {
      if (arguments->elements.size() == this->arguments->elements.size()) {
        for (unsigned int i = 0; i < args->elements.size(); i++) {
          closure->set(arguments->elements[i], args->elements[i]);
        }
      } else {
        return to_obj(Runtime::ret_exception(
            "Funcion <" + calling_env->get_key(shared_from_this()) +
            ">: wrong number of parameters"));
      }
    }
    return EVAL(expression, closure);
  }
}

// QUICK CONSTRUCTORS

shared_ptr<Atom> atom(shared_ptr<Object> o) { return make_shared<Atom>(o); }
shared_ptr<Exception> exception(std::string message) {
  return make_shared<Exception>(message);
}
shared_ptr<Nil> nil() { return make_shared<Nil>(); }
shared_ptr<Symbol> symbol(std::string s) { return make_shared<Symbol>(s); }
shared_ptr<Bool> boolean(bool b) { return make_shared<Bool>(b); }
shared_ptr<Keyword> keyword(std::string s) { return make_shared<Keyword>(s); }
shared_ptr<Str> str(std::string s) { return make_shared<Str>(s); }
shared_ptr<Number> number(double n) { return make_shared<Number>(n); }
shared_ptr<List> list() { return make_shared<List>(); }
shared_ptr<Vec> vec() { return make_shared<Vec>(); }
shared_ptr<Dict> dict() { return make_shared<Dict>(); }
shared_ptr<Signal> signal(INNER_SIGNALS v) { return make_shared<Signal>(v); }
shared_ptr<Function> func(std::function<shared_ptr<Object>(shared_ptr<List>)> f,
                          std::string name, std::string help) {
  return make_shared<Function>(f, name, help);
}
shared_ptr<Function> func(shared_ptr<List> arguments,
                          shared_ptr<Object> expression,
                          shared_ptr<Environment> env, std::string name,
                          std::string help, bool is_macro) {
  return make_shared<Function>(arguments, expression, env, name, help,
                               is_macro);
}

// CONVERSIONS

shared_ptr<Atom> to_atom(shared_ptr<Object> o) {
  return std::static_pointer_cast<Atom>(o);
}

shared_ptr<Exception> to_exception(shared_ptr<Object> o) {
  return std::static_pointer_cast<Exception>(o);
}

shared_ptr<Nil> to_nil(shared_ptr<Object> o) {
  return std::static_pointer_cast<Nil>(o);
}

shared_ptr<Symbol> to_symbol(shared_ptr<Object> o) {
  return std::static_pointer_cast<Symbol>(o);
}

shared_ptr<Bool> to_bool(shared_ptr<Object> o) {
  return std::static_pointer_cast<Bool>(o);
}

shared_ptr<Keyword> to_keyword(shared_ptr<Object> o) {
  return std::static_pointer_cast<Keyword>(o);
}

shared_ptr<Signal> to_signal(shared_ptr<Object> o) {
  return std::static_pointer_cast<Signal>(o);
}

shared_ptr<Number> to_number(shared_ptr<Object> o) {
  return std::static_pointer_cast<Number>(o);
}

shared_ptr<Str> to_str(shared_ptr<Object> o) {
  return std::static_pointer_cast<Str>(o);
}

shared_ptr<List> to_list(shared_ptr<Object> o) {
  return std::static_pointer_cast<List>(o);
}

shared_ptr<Vec> to_vec(shared_ptr<Object> o) {
  return std::static_pointer_cast<Vec>(o);
}

shared_ptr<Dict> to_dict(shared_ptr<Object> o) {
  return std::static_pointer_cast<Dict>(o);
}

shared_ptr<Function> to_function(shared_ptr<Object> o) {
  return std::static_pointer_cast<Function>(o);
}

#ifdef DEBUG_Types_info
void type_info(shared_ptr<Object> obj, std::string msg) {
  std::string type;
  switch (obj->type) {
  case NIL:
    type = "nil";
    break;
  case SYMBOL:
    type = "symbol";
    break;
  case KEYWORD:
    type = "keyword";
    break;
  case NUMBER:
    type = "number";
    break;
  case STRING:
    type = "str";
    break;
  case SIGNAL:
    type = "signal";
    break;
  case FUNCTION:
    type = "function";
    break;
  case LIST:
    type = "list";
    break;
  case VEC:
    type = "vec";
    break;
  case DICT:
    type = "dict";
    break;
  case ENVIRONMENT:
    type = "environment";
    break;
  }

  cout << "TYPE: " << type << endl << "VALUE: " << print_element(obj) << endl;
}
#endif
} // namespace ml

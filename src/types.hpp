#pragma once
#include "debug.hpp"
#include "inner_signals.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
using std::shared_ptr, std::vector;

namespace ml {

enum OBJECT_TYPE {
  ROOT,
  ATOM,
  EXCEPTION,
  SIGNAL,
  NIL,
  ENVIRONMENT,
  FUNCTION,
  SYMBOL,
  BOOL,
  KEYWORD,
  NUMBER,
  STRING,
  LIST,
  VEC,
  DICT,
};

// OBJECT

class Object {
public:
  Object(OBJECT_TYPE o_type);
  OBJECT_TYPE type;
  bool is_macro = false;
};

// ROOT

class Root : public Object {
public:
  Root();
  vector<shared_ptr<Object>> expressions;
};

// ATOM

class Atom : public Object {
public:
  Atom(shared_ptr<Object> o);
  void set(shared_ptr<Object> o);
  shared_ptr<Object> value();
  OBJECT_TYPE value_type();

private:
  shared_ptr<Object> content;
};

// EXCEPTION

class Exception : public Object {
public:
  Exception(std::string message);
  std::string value() const;

private:
  std::string message;
};

// SIGNAL

class Signal : public Object {
public:
  Signal(INNER_SIGNALS value);
  INNER_SIGNALS _value;
};

// NIL

class Nil : public Object {
public:
  Nil();
};

// SYMBOL

class Symbol : public Object {
public:
  Symbol(std::string value);
  const std::string &value() const;

private:
  std::string _value;
};

// BOOL

class Bool : public Object {
public:
  Bool(bool value);
  const bool value() const;
  const bool operator==(const shared_ptr<Bool> other);

private:
  bool _value;
};

// KEYWORD

class Keyword : public Object {
public:
  Keyword(std::string value);
  const std::string &value() const;

private:
  std::string _value;
};

// NUMBER

class Number : public Object {
public:
  Number(double value);
  const double value() const;
  const bool operator==(const shared_ptr<Number> other);

private:
  double _value;
};

// STR

class Str : public Object {
public:
  Str(std::string value);
  const std::string &value() const;
  const bool operator==(const shared_ptr<Str> other);

private:
  std::string _value;
};

// LIST

class List : public Object {
public:
  List();
  shared_ptr<Object> operator[](unsigned int index);
  void append(shared_ptr<Object> obj);
  vector<shared_ptr<Object>> elements;
  shared_ptr<Object> meta;
  const bool operator==(const shared_ptr<List> other);
};

// VEC

class Vec : public Object {
public:
  Vec();
  shared_ptr<Object> operator[](unsigned int index);
  void append(shared_ptr<Object> obj);
  vector<shared_ptr<Object>> elements;
  shared_ptr<Object> meta;
  const bool operator==(const shared_ptr<Vec> other);
};

// DICT

class Dict : public Object {
public:
  Dict();
  shared_ptr<Object> operator[](shared_ptr<Object> key);
  void append(shared_ptr<Object> key, shared_ptr<Object> value);
  std::unordered_map<shared_ptr<Object>, shared_ptr<Object>> map;
  shared_ptr<Object> meta;
  const bool operator==(const shared_ptr<Dict> other);
};

// FUNCTION

class Environment;
class Function : public Object, public std::enable_shared_from_this<Function> {
public:
  Function(std::function<shared_ptr<Object>(shared_ptr<List>)> f,
           std::string name, std::string help);
  Function(shared_ptr<List> arguments, shared_ptr<Object> expression,
           shared_ptr<Environment> env, std::string name, std::string help,
           bool is_macro = false);
  shared_ptr<Object> call(shared_ptr<List> args);
  bool compiled;
  std::string name;
  shared_ptr<List> arguments;
  shared_ptr<Object> expression;
  shared_ptr<Environment> calling_env;
  int last_is_variadic = -1;
  shared_ptr<Object> meta;

private:
  std::function<shared_ptr<Object>(shared_ptr<List>)> f;
};

// INSTANTIATIONS
shared_ptr<Nil> nil();
shared_ptr<Atom> atom(shared_ptr<Object> o);
shared_ptr<Exception> exception(std::string message);
shared_ptr<Symbol> symbol(std::string s);
shared_ptr<Bool> boolean(bool b);
shared_ptr<Keyword> keyword(std::string s);
shared_ptr<Number> number(double n);
shared_ptr<Str> str(std::string s);
shared_ptr<Signal> signal(INNER_SIGNALS v);
shared_ptr<List> list();
shared_ptr<Vec> vec();
shared_ptr<Dict> dict();
shared_ptr<Function> func(std::function<shared_ptr<Object>(shared_ptr<List>)>,
                          std::string name = "", std::string help = "");
shared_ptr<Function> func(shared_ptr<List> arguments,
                          shared_ptr<Object> expression,
                          shared_ptr<Environment> env, std::string name,
                          std::string help = "", bool is_macro = false);

// CONVERSIONS

template <typename T> shared_ptr<Object> to_obj(T t) {
  return std::static_pointer_cast<Object>(t);
}
shared_ptr<Atom> to_atom(shared_ptr<Object> o);
shared_ptr<Exception> to_exception(shared_ptr<Object> o);
shared_ptr<Nil> to_nil(shared_ptr<Object> o);
shared_ptr<Symbol> to_symbol(shared_ptr<Object> o);
shared_ptr<Bool> to_bool(shared_ptr<Object> o);
shared_ptr<Keyword> to_keyword(shared_ptr<Object> o);
shared_ptr<Number> to_number(shared_ptr<Object> o);
shared_ptr<Str> to_str(shared_ptr<Object> o);
shared_ptr<Signal> to_signal(shared_ptr<Object> o);
shared_ptr<List> to_list(shared_ptr<Object> o);
shared_ptr<Vec> to_vec(shared_ptr<Object> o);
shared_ptr<Dict> to_dict(shared_ptr<Object> o);
shared_ptr<Function> to_function(shared_ptr<Object> o);

template <typename T, typename D> shared_ptr<T> to(shared_ptr<D> o) {
  return std::static_pointer_cast<T>(to_obj(o));
}

#ifdef DEBUG_Types_info
void type_info(shared_ptr<Object> obj, std::string msg = "");
#endif

} // namespace ml

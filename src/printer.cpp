#include "printer.hpp"
#include "types.hpp"
#include <iostream>
using std::cout, std::endl;

namespace ml {
string print_element(shared_ptr<Object> element) {
  switch (element->type) {
  case NUMBER:
    return std::to_string(to_number(element)->value());
  case STRING:
    return print_string(to_str(element));
  case SYMBOL:
    return to_symbol(element)->value();
  case BOOL:
    return print_bool(to_bool(element));
  case KEYWORD:
    return to_keyword(element)->value();
  case LIST:
    return print_list(to_list(element));
  case VEC:
    return print_vec(to_vec(element));
  case DICT:
    return print_dict(to_dict(element));
  case NIL:
    return print_nil(to_nil(element));
  case FUNCTION:
    return "FUNCTION";
  default:
    return "nil";
  }
}

string print_bool(shared_ptr<Bool> b) {
  if (b->value())
    return "<true>";
  else
    return "<false>";
}

string print_list(shared_ptr<List> list) {
  string ret = "( ";
  for (auto el : list->elements)
    ret += print_element(el) + " ";
  ret += ")";
  return ret;
}

string print_nil(shared_ptr<Object> nil_o) { return "nil"; }

string print_vec(shared_ptr<Vec> vector) {
  string ret = "[ ";
  for (auto el : vector->elements)
    ret += print_element(el) + " ";
  ret += "]";
  return ret;
}

string print_dict(shared_ptr<Dict> dict) {
  string ret = "{ ";
  for (auto pair : dict->map)
    ret += print_element(pair.first) + " : " + print_element(pair.second) + " ";
  ret += "}";
  return ret;
}

string print_symbol(shared_ptr<Symbol> symbol) { return symbol->value(); }

string print_number(shared_ptr<Number> number) {
  return std::to_string(number->value());
}

string print_string(shared_ptr<Str> str, bool print_readably) {
  if (print_readably) {
    string ret;
    ret.reserve(str->value().length());
    for (unsigned int i = 0; i < str->value().length(); i++) {
      char ch = str->value().at(i);
      if (ch == '\\' and i < str->value().length() - 1) {
        switch (char n = str->value().at(i + 1)) {
        case '\\':
          ret.push_back('\\');
          break;
        case 'n':
          ret.push_back('\n');
          break;
        case 't':
          ret.push_back('\t');
          break;
        case '"':
          ret.push_back('"');
          break;
        case 'r':
          ret.push_back('\r');
          break;
        default:
          ret.push_back('\\');
        }
        i++;
      } else {
        ret.push_back(ch);
      }
    }
    return ret;
  } else {
    return str->value();
  }
}

string print_keyword(shared_ptr<Keyword> keyword) { return keyword->value(); }

string debug_object(shared_ptr<Object> obj, unsigned int level) {
  auto tabs = [](unsigned int n) {
    string t;
    for (unsigned int i = 0; i < n; i++)
      t += '\t';
    return t;
  };

  string ret;
  switch (obj->type) {
  case ATOM:
    ret += tabs(level) + "ATOM  ->\n" + debug_object(to_atom(obj)->value());
    break;
  case LIST:
    ret += tabs(level) + "LIST: (\n";
    for (auto el : to_list(obj)->elements) {
      ret += tabs(level + 1) + debug_object(el, level + 1) + "\n";
    }
    ret += tabs(level) + ")\n";
    break;
  case SIGNAL:
    ret += tabs(level) + std::to_string(to_signal(obj)->_value) + '\n';
    break;
  case NIL:
    ret += tabs(level) + "NIL" + '\n';
    break;
  case ENVIRONMENT:
    ret += tabs(level) + "ENVIRONMENT" + '\n';
    break;
  case FUNCTION:
    ret += tabs(level) + "FUNCTION" + '\n';
    break;
  case SYMBOL:
    ret += tabs(level) + "SYMBOL: " + to_symbol(obj)->value() + '\n';
    break;
  case BOOL:
    ret += tabs(level) +
           "BOOL: " + (to_bool(obj)->value() ? "<TRUE>" : "FALSE") + '\n';
    break;
  case KEYWORD:
    ret += tabs(level) + "KEYWORD: " + to_keyword(obj)->value() + '\n';
    break;
  case NUMBER:
    ret += tabs(level) + "NUMBER: " + std::to_string(to_number(obj)->value()) +
           '\n';
    break;
  case STRING:
    ret += tabs(level) + "STRING: " + to_str(obj)->value() + '\n';
    break;
  case VEC:
    ret += tabs(level) + "VEC: [\n";
    for (auto el : to_vec(obj)->elements) {
      ret += tabs(level + 1) + debug_object(el, level + 1) + "\n";
    }
    ret += tabs(level) + "]\n";
    break;
  case DICT:
    ret += tabs(level) + "DICT: {\n";
    for (auto el : to_dict(obj)->map) {
      string key = el.first->type == STRING ? to_str(el.first)->value()
                                            : to_keyword(el.first)->value();
      auto value = el.second;
      ret += tabs(level + 1) + key + ": " + debug_object(el.second, level + 1) +
             "\n";
    }
    ret += tabs(level) + "}\n";
    break;
  }
  return ret;
}

} // namespace ml

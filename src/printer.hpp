#pragma once
#include "types.hpp"
using std::string;

namespace ml {

string print_element(shared_ptr<Object> element);
string print_nil(shared_ptr<Object> nil_o);
string print_bool(shared_ptr<Bool> b);
string print_list(shared_ptr<List> list);
string print_vec(shared_ptr<Vec> vector);
string print_dict(shared_ptr<Dict> dict);
string print_symbol(shared_ptr<Symbol> symbol);
string print_number(shared_ptr<Number> number);
string print_string(shared_ptr<Str> str, bool print_readably = true);
string print_keyword(shared_ptr<Keyword> keyword);

string debug_object(shared_ptr<Object> obj, unsigned int level = 0);
} // namespace ml

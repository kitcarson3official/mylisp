#include "core.hpp"
#include "parser.hpp"
#include "printer.hpp"
#include "repl.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
using std::endl, std::cout;

namespace ml {

shared_ptr<Environment> initialize() {
  shared_ptr<Environment> core = std::make_shared<Environment>();

  core->set(str("quit"), func([](shared_ptr<List> args) {
              Runtime::message_signal = signal(QUIT);
              return nil();
            }));

  core->set(str("true"), boolean(true));

  core->set(str("false"), boolean(false));

  core->set(str("nil?"), func([](shared_ptr<List> args) {
              if (args->elements[0]->type == NIL)
                return boolean(true);
              else
                return boolean(false);
            }));

  core->set(str("symbol?"), func([](shared_ptr<List> args) {
              if (args->elements[0]->type == SYMBOL)
                return boolean(true);
              else
                return boolean(false);
            }));

  core->set(str("keyword?"), func([](shared_ptr<List> args) {
              if (args->elements[0]->type == KEYWORD)
                return boolean(true);
              else
                return boolean(false);
            }));

  core->set(str("vector?"), func([](shared_ptr<List> args) {
              if (args->elements[0]->type == VEC)
                return boolean(true);
              else
                return boolean(false);
            }));

  core->set(str("sequential?"), func([](shared_ptr<List> args) {
              if (args->elements[0]->type == VEC or
                  args->elements[0]->type == LIST)
                return boolean(true);
              else
                return boolean(false);
            }));

  core->set(str("map?"), func([](shared_ptr<List> args) {
              if (args->elements[0]->type == DICT)
                return boolean(true);
              else
                return boolean(false);
            }));

  core->set(str("true?"), func([](shared_ptr<List> args) {
              if (args->elements[0]->type == BOOL)
                return to_obj(boolean(to_bool(args->elements[0])->value()));
              else
                return to_obj(exception("true?: bad parameter passed"));
            }));

  core->set(str("false?"), func([](shared_ptr<List> args) {
              if (args->elements[0]->type == BOOL)
                return to_obj(boolean(not to_bool(args->elements[0])->value()));
              else
                return to_obj(exception("true?: bad parameter passed"));
            }));

  core->set(str("eval"), func([core](shared_ptr<List> args) {
              if (args->elements.size() == 1)
                return EVAL(args->elements[0], core);
              else {
                shared_ptr<List> ret = list();
                for (auto el : args->elements)
                  ret->append(EVAL(el, core));
                return to_obj(nil());
              }
            }));
  core->set(str("throw"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 1) {
                Runtime::unhandled_exc = args->elements[0];
                return to_obj(nil());
              } else {
                return to_obj(exception("throw: bad parameter passed"));
              }
            }));

  core->set(
      str("apply"), func([](shared_ptr<List> args) {
        if (args->elements.size() > 0 and args->elements[0]->type == FUNCTION) {
          shared_ptr<List> fargs = list();
          for (unsigned int i = 1; i < args->elements.size() - 1; i++) {
            if (args->elements[i]->type == LIST) {
              shared_ptr<List> tmp_list = to_list(args->elements[i]);
              for (unsigned int i = 0; i < tmp_list->elements.size(); i++)
                fargs->append(tmp_list->elements[i]);
            } else if (args->elements[i]->type == VEC) {
              shared_ptr<Vec> tmp_vec = to_vec(args->elements[i]);
              for (unsigned int i = 0; i < tmp_vec->elements.size(); i++)
                fargs->append(tmp_vec->elements[i]);
            } else {
              fargs->append(args->elements[i]);
            }
          }
          shared_ptr<Object> last_element =
              args->elements[args->elements.size() - 1];
          if (last_element->type == LIST) {
            for (auto el : to_list(last_element)->elements)
              fargs->append(el);
          } else if (last_element->type == VEC) {
            for (auto el : to_vec(last_element)->elements)
              fargs->append(el);
          } else {
            fargs->append(last_element);
          }
          return to_function(args->elements[0])->call(fargs);
        } else {
          shared_ptr<Exception> ret = exception("apply: bad parameter passed");
          Runtime::unhandled_exc = ret;
          return to_obj(exception("apply: bad parameter passed"));
        }
      }));

  core->set(str("map"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 2 and
                  args->elements[0]->type == FUNCTION and
                  (args->elements[1]->type == LIST or
                   args->elements[1]->type == VEC)) {
                shared_ptr<List> ret = list();
                if (args->elements[1]->type == LIST) {
                  shared_ptr<List> tmp_list = to_list(args->elements[1]);
                  for (auto el : tmp_list->elements) {
                    shared_ptr<List> fargs = list();
                    fargs->append(el);
                    ret->append(to_function(args->elements[0])->call(fargs));
                  }
                } else {
                  shared_ptr<Vec> tmp_vec = to_vec(args->elements[1]);
                  for (auto el : tmp_vec->elements) {
                    shared_ptr<List> fargs = list();
                    fargs->append(el);
                    ret->append(to_function(args->elements[0])->call(fargs));
                  }
                }
                return to_obj(ret);
              } else {
                return to_obj(exception("map: bad parameter passed"));
              }
            }));

  core->set(str("read-string"), func([](shared_ptr<List> args) {
              Parser p;
              if (args->elements.size() > 0) {
                for (unsigned int i = 0; i < args->elements.size(); i++) {
                  shared_ptr<Object> el = args->elements[i];
                  if (el->type == STRING) {
                    shared_ptr<Object> ret = p.parse(to_str(el)->value());
                    if (i == args->elements.size() - 1) {
                      return ret;
                    }
                  } else {
                    return to_obj(nil());
                  }
                }
                return to_obj(nil());
              } else {
                return to_obj(nil());
              }
            }));

  core->set(str("slurp"), func([](shared_ptr<List> args) {
              if (args->elements.size() > 0) {
                if (args->elements[0]->type == STRING) {
                  string filename = to_str(args->elements[0])->value();
                  std::ifstream ifs(filename);
                  if (ifs.is_open()) {
                    std::stringstream buffer;
                    buffer << ifs.rdbuf();
                    return to_obj(str(buffer.str()));
                  } else {
                    cout << "slurp: FILE IO ERROR" << endl;
                    return to_obj(nil());
                  }
                } else {
                  return to_obj(nil());
                }
              } else {
                return to_obj(nil());
              }
            }));

  core->set(str("+"), func([](shared_ptr<List> args) {
              double sum = 0;
              if (args->elements.size() > 0) {
                for (auto el : args->elements) {
                  if (el->type == NUMBER)
                    sum += to_number(el)->value();
                  else {
                    cout << "invalid argument of + operator " +
                                print_element(el)
                         << endl;
                    return to_obj(nil());
                  }
                }
                return to_obj(number(sum));
              } else {
                cout << "+ operator called without arguments" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("-"), func([](shared_ptr<List> args) {
              double tot = 0;
              if (args->elements.size() > 0) {
                if (args->elements[0]->type == NUMBER) {
                  tot += to_number(args->elements[0])->value();
                  for (unsigned int i = 1; i < args->elements.size(); i++) {
                    shared_ptr<Object> el = args->elements[i];
                    if (el->type == NUMBER)
                      tot -= to_number(el)->value();
                    else {
                      cout << "invalid argument of + operator " +
                                  print_element(el)
                           << endl;
                      return to_obj(nil());
                    }
                  }
                  return to_obj(number(tot));
                } else {
                  cout << "invalid argument of + operator " +
                              print_element(args->elements[0])
                       << endl;
                  return to_obj(nil());
                }
              } else {
                cout << "+ operator called without arguments" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("*"), func([](shared_ptr<List> args) {
              double top = 1;
              if (args->elements.size() > 0) {
                for (auto el : args->elements) {
                  if (el->type == NUMBER)
                    top *= to_number(el)->value();
                  else {
                    cout << "invalid argument of + operator " +
                                print_element(el)
                         << endl;
                    return to_obj(nil());
                  }
                }
                return to_obj(number(top));
              } else {
                cout << "+ operator called without arguments" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("/"), func([](shared_ptr<List> args) {
              double tot = 0;
              if (args->elements.size() > 0) {
                if (args->elements[0]->type == NUMBER) {
                  tot += to_number(args->elements[0])->value();
                  for (auto el : args->elements) {
                    if (el->type == NUMBER) {
                      if (to_number(el)->value() != 0)
                        tot /= to_number(el)->value();
                      else {
                        cout << "DIVISION BY 0" << endl;
                        return to_obj(nil());
                      }
                    } else {
                      cout << "invalid argument of + operator " +
                                  print_element(el)
                           << endl;
                      return to_obj(nil());
                    }
                  }
                  return to_obj(number(tot));
                } else {
                  cout << "invalid argument of + operator " +
                              print_element(args->elements[0])
                       << endl;
                  return to_obj(nil());
                }
              } else {
                cout << "+ operator called without arguments" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("println"), func([](shared_ptr<List> args) {
              if (args->elements.size() > 0) {
                for (auto el : args->elements) {
                  if (el->type == STRING)
                    cout << print_string(to_str(el), false) << " ";
                  else
                    cout << print_element(args->elements[0]) << " ";
                }
                cout << endl;
              } else
                cout << "prn: pass at least one argument" << endl;
              return nil();
            }));

  core->set(str("prn"), func([](shared_ptr<List> args) {
              if (args->elements.size() > 0) {
                for (auto el : args->elements) {
                  if (el->type == STRING)
                    cout << print_string(to_str(el), true) << " ";
                  else
                    cout << print_element(args->elements[0]) << " ";
                }
                cout << endl;
              } else
                cout << "prn: pass at least one argument" << endl;
              return nil();
            }));

  core->set(str("pr-str"), func([](shared_ptr<List> args) {
              string ret;
              if (args->elements.size() > 0) {
                for (auto el : args->elements) {
                  if (el->type == STRING)
                    ret += print_string(to_str(el), true) + " ";
                  else
                    ret += print_element(args->elements[0]) + " ";
                }
                return str(ret);
              } else
                cout << "prn: pass at least one argument" << endl;
              return to_str(nil());
            }));

  core->set(str("str"), func([](shared_ptr<List> args) {
              string ret;
              if (args->elements.size() > 0) {
                for (auto el : args->elements) {
                  if (el->type == STRING)
                    ret += print_string(to_str(el), false);
                  else
                    ret += print_element(args->elements[0]);
                }
                return str(ret);
              } else
                cout << "prn: pass at least one argument" << endl;
              return to_str(nil());
            }));

  core->set(str("atom"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 1) {
                return to_obj(atom(args->elements[0]));
              } else {
                cout << "atom: pass only one argument" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("atom?"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 1) {
                return args->elements[0]->type == ATOM ? to_obj(boolean(true))
                                                       : boolean(false);
              } else {
                cout << "atom?: pass only one argument" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("deref"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 1) {
                if (args->elements[0]->type == ATOM) {
                  return to_atom(args->elements[0])->value();

                } else {
                  cout << "deref: argument passed is not an Atom" << endl;
                  return to_obj(nil());
                }
              } else {
                cout << "deref: pass only one argument" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("reset!"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 2) {
                if (args->elements[0]->type == ATOM) {
                  to_atom(args->elements[0])->set(args->elements[1]);
                  return args->elements[1];
                } else {
                  cout << "reset!: first argument passed is not an Atom"
                       << endl;
                  return to_obj(nil());
                }
              } else {
                cout << "reset!: pass two argument" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("swap!"), func([](shared_ptr<List> args) {
              if (args->elements.size() > 2) {
                if (args->elements[0]->type == ATOM and
                    args->elements[1]->type == FUNCTION) {
                  shared_ptr<List> fargs = list();
                  fargs->append(to_atom(args->elements[0])->value());
                  for (unsigned int i = 2; i < args->elements.size(); i++) {
                    fargs->append(args->elements[i]);
                  }
                  shared_ptr<Object> val =
                      to_function(args->elements[1])->call(fargs);
                  to_atom(args->elements[0])->set(val);
                  return val;
                } else {
                  cout << "swap!: first argument passed must be an Atom"
                          " and second argument must be a function"
                       << endl;
                  return to_obj(nil());
                }
              } else {
                cout << "swap!: pass at least an atom and a function" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("symbol"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 1 and
                  args->elements[0]->type == STRING) {
                return to_obj(symbol(to_str(args->elements[0])->value()));
              } else {
                return to_obj(exception("symbol: bad parameter passed"));
              }
            }));

  core->set(
      str("keyword"), func([](shared_ptr<List> args) {
        if (args->elements.size() == 1 and args->elements[0]->type == STRING) {
          return to_obj(keyword(":" + to_str(args->elements[0])->value()));
        } else if (args->elements.size() == 1 and
                   args->elements[0]->type == KEYWORD)
          return to_obj(args->elements[0]);
        else
          return to_obj(exception("keyword: bad parameter passed"));
      }));

  // TODO

  core->set(str("list"), func([](shared_ptr<List> args) {
              shared_ptr<List> ret = list();
              for (auto el : args->elements)
                ret->append(el);
              return ret;
            }));

  core->set(str("list?"), func([](shared_ptr<List> args) {
              if (args->elements.size() > 0 and args->elements[0]->type == LIST)
                return boolean(true);
              else
                return boolean(false);
            }));

  core->set(
      str("hash-map"), func([](shared_ptr<List> args) {
        shared_ptr<Dict> ret;
        if (args->elements.size() % 2 == 0) {
          bool valid = true;
          for (unsigned int i = 0; i < args->elements.size(); i += 2) {
            if (args->elements[i]->type == SYMBOL) {
              ret->append(to_symbol(args->elements[i]), args->elements[i + 1]);
            } else {
              return to_obj(exception("hash-map: error, " +
                                      to_symbol(args->elements[i])->value() +
                                      " is not a symbol"));
            }
          }
        } else {
          return to_obj(exception("hash-map: bad number of parameters"));
        }
        return to_obj(ret);
      }));

  core->set(
      str("nth"), func([](shared_ptr<List> args) {
        if (args->elements.size() == 2 and args->elements[0]->type == NUMBER and
            (args->elements[1]->type == LIST or
             args->elements[1]->type == VEC)) {
          if (args->elements[1]->type == LIST) {
            if (to_number(args->elements[0])->value() <
                to_list(args->elements[1])->elements.size()) {
              return to_list(args->elements[1])
                  ->elements[static_cast<int>(
                      to_number(args->elements[0])->value())];
            } else {
              cout << "nth: out of bounds of list" << endl;
              exit(1);
            }
          }
          if (args->elements[1]->type == VEC) {
            if (to_number(args->elements[0])->value() <
                to_vec(args->elements[1])->elements.size()) {
              return to_vec(args->elements[1])
                  ->elements[static_cast<int>(
                      to_number(args->elements[0])->value())];
            } else {
              cout << "nth: out of bounds of list" << endl;
              exit(1);
            }
          }
          return to_obj(nil());
        } else {
          cout << "nth: takes two parameters, an integer and a list (or vec)"
               << endl;
          exit(1);
        }
      }));

  core->set(str("first"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 1 and
                  args->elements[0]->type == LIST) {
                if (not to_list(args->elements[0])->elements.empty())
                  return to_list(args->elements[0])->elements[0];
                else
                  return to_obj(nil());
              } else if (args->elements.size() == 1 and
                         args->elements[0]->type == VEC) {
                if (not to_vec(args->elements[0])->elements.empty())
                  return to_vec(args->elements[0])->elements[0];
                else
                  return to_obj(nil());
              } else {
                cout << "first: expect one paramenter (list or vec)" << endl;
                exit(1);
              }
            }));

  core->set(str("rest"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 1 and
                  args->elements[0]->type == LIST) {
                shared_ptr<List> ret = list();
                if (not to_list(args->elements[0])->elements.empty()) {
                  for (unsigned int i = 1;
                       i < to_list(args->elements[0])->elements.size(); i++)
                    ret->append(to_list(args->elements[0])->elements[i]);
                }
                return ret;
              } else if (args->elements.size() == 1 and
                         args->elements[0]->type == VEC) {
                shared_ptr<List> ret = list();
                if (not to_vec(args->elements[0])->elements.empty()) {
                  for (unsigned int i = 1;
                       i < to_vec(args->elements[0])->elements.size(); i++)
                    ret->append(to_vec(args->elements[0])->elements[i]);
                }
                return ret;
              } else {
                cout << "rest: expect one paramenter (list or vec)" << endl;
                exit(1);
              }
            }));

  core->set(str("vec"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 1) {
                if (args->elements[0]->type == LIST) {
                  shared_ptr<Vec> ret_vec = vec();
                  for (auto el : to_list(args->elements[0])->elements)
                    ret_vec->append(el);
                  return to_obj(ret_vec);
                } else if (args->elements[0]->type == VEC) {
                  return to_obj(args->elements[0]);
                } else {
                  cout << "vec: only list or vec are valid arguments" << endl;
                  return to_obj(nil());
                }
              } else {
                cout << "vec: too many arguments" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("vector"), func([](shared_ptr<List> args) {
              shared_ptr<Vec> ret;
              for (auto el : args->elements)
                ret->append(el);
              return ret;
            }));

  core->set(str("cons"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 2 and
                  args->elements[1]->type == LIST) {
                shared_ptr<List> new_list = list();
                new_list->append(args->elements[0]);
                for (auto el : to_list(args->elements[1])->elements)
                  new_list->append(el);
                return to_obj(new_list);
              } else if (args->elements.size() == 2 and
                         args->elements[1]->type == VEC) {
                shared_ptr<List> new_list = list();
                new_list->append(args->elements[0]);
                for (auto el : to_vec(args->elements[1])->elements)
                  new_list->append(el);
                return to_obj(new_list);
              } else {
                cout << "cons: needs two parameters and the second one must be "
                        "a list or a vector"
                     << endl;
                exit(1);
                return to_obj(nil());
              }
            }));

  core->set(str("concat"), func([](shared_ptr<List> args) {
              bool valid = true;
              for (auto el : args->elements) {
                if (el->type != LIST and el->type != VEC) {
                  valid = false;
                  break;
                }
              }
              if (valid) {
                shared_ptr<List> new_list = list();
                for (auto obj_l : args->elements) {
                  if (obj_l->type == LIST) {
                    shared_ptr<List> l = to_list(obj_l);
                    for (auto el : l->elements)
                      new_list->append(el);
                  } else if (obj_l->type == VEC) {
                    shared_ptr<Vec> l = to_vec(obj_l);
                    for (auto el : l->elements)
                      new_list->append(el);
                  }
                }
                return to_obj(new_list);
              } else {
                cout << "concat: all parameters must be lists" << endl;
                exit(1);
                return to_obj(nil());
              }
            }));

  core->set(str("empty?"), func([](shared_ptr<List> args) {
              if (args->elements.size() > 0 and
                  args->elements[0]->type == LIST) {
                if (to_list(args->elements[0])->elements.empty())
                  return boolean(true);
                else
                  return boolean(false);
              } else {
                cout << "empty?: pass a list as first parameter" << endl;
                return boolean(false);
              }
            }));

  core->set(str("count"), func([](shared_ptr<List> args) {
              if (args->elements.size() > 0 and
                  args->elements[0]->type == LIST) {
                return number(to_list(args->elements[0])->elements.size());
              } else {
                cout << "empty?: pass a list as first parameter" << endl;
                return to_number(nil());
              }
            }));

  core->set(str("="), func([](shared_ptr<List> args) {
              if (args->elements.size() == 2 and
                  args->elements[0]->type == args->elements[1]->type) {
                shared_ptr<Object> o0 = args->elements[0],
                                   o1 = args->elements[1];
                switch (o0->type) {
                case BOOL:
                  return boolean(*to_bool(o1) == to_bool(o0));
                case NUMBER:
                  return boolean(*to_number(o1) == to_number(o0));
                case STRING:
                  return boolean(*to_str(o1) == to_str(o0));
                case LIST:
                  return boolean(*to_list(o1) == to_list(o0));
                case DICT:
                  return boolean(*to_dict(o1) == to_dict(o0));
                case VEC:
                  return boolean(*to_vec(o1) == to_vec(o0));
                default:
                  return to_bool(nil());
                }
              } else {
                return boolean(false);
              }
            }));

  core->set(str(">"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 2 and
                  args->elements[0]->type == NUMBER and
                  args->elements[1]->type == NUMBER) {
                if (to_number(args->elements[0])->value() >
                    to_number(args->elements[1])->value())
                  return to_obj(boolean(true));
                else
                  return to_obj(boolean(false));
              } else {
                cout << "> operator must be called with two numbers" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("<"), func([](shared_ptr<List> args) {
              if (args->elements.size() == 2 and
                  args->elements[0]->type == NUMBER and
                  args->elements[1]->type == NUMBER) {
                if (to_number(args->elements[0])->value() <
                    to_number(args->elements[1])->value())
                  return to_obj(boolean(true));
                else
                  return to_obj(boolean(false));
              } else {
                cout << "< operator must be called with two numbers" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str(">="), func([](shared_ptr<List> args) {
              if (args->elements.size() == 2 and
                  args->elements[0]->type == NUMBER and
                  args->elements[1]->type == NUMBER) {
                if (to_number(args->elements[0])->value() >=
                    to_number(args->elements[1])->value())
                  return to_obj(boolean(true));
                else
                  return to_obj(boolean(false));
              } else {
                cout << ">= operator must be called with two numbers" << endl;
                return to_obj(nil());
              }
            }));

  core->set(str("<="), func([](shared_ptr<List> args) {
              if (args->elements.size() == 2 and
                  args->elements[0]->type == NUMBER and
                  args->elements[1]->type == NUMBER) {
                if (to_number(args->elements[0])->value() <=
                    to_number(args->elements[1])->value())
                  return to_obj(boolean(true));
                else
                  return to_obj(boolean(false));
              } else {
                cout << "<= operator must be called with two numbers" << endl;
                return to_obj(nil());
              }
            }));

  rep("(def! not (fn* (a) (if a false true)))", core);

  rep(R"(
  (def! load-file (fn* (file)
    (eval (read-string (slurp file)))
  ))
  )",
      core);
  return core;
}

shared_ptr<Environment> get_builtin() { return initialize(); }
} // namespace ml

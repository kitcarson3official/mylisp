#include "repl.hpp"
#include "core.hpp"
#include "debug.hpp"
#include "parser.hpp"
#include "printer.hpp"
#include "types.hpp"
#include <functional>
#include <iostream>
#include <memory>
using std::cout, std::endl;

namespace ml {

shared_ptr<Object> Runtime::message_signal = nil();
shared_ptr<Object> Runtime::unhandled_exc = nil();
shared_ptr<Object> Runtime::current_env = nil();

bool catching = false;

Runtime::Runtime() {
  running = true;
  core_env = get_builtin();
  Runtime::current_env = core_env;
}

const vector<std::string> keywords = {
    "fn*",         "if",          "do",    "let*",       "def!",
    "defmacro!",   "expandmacro", "quote", "quasiquote", "quasiquoteexpand",
    "macroexpand", "try*",        "catch*"};

shared_ptr<Environment> Runtime::env() { return core_env; }

std::string rep(std::string input, shared_ptr<Environment> rep_env) {
  return PRINT(EVAL(READ(input), rep_env));
}

void check_exc() {
  // if (Runtime::unhandled_exc->type != NIL and not catching) {
  if (Runtime::unhandled_exc->type == EXCEPTION) {
    cout << "----------------------------------" << endl;
    cout << "there is an unhandled exception" << endl;
    cout << debug_object(Runtime::unhandled_exc) << endl;
    cout << "----------------------------------" << endl;
    exit(1);
  }
}

shared_ptr<Exception> Runtime::ret_exception(string message) {
  shared_ptr<Exception> exc = exception(message);
  Runtime::unhandled_exc = exc;
  return exc;
}

shared_ptr<Object> READ(std::string input) {
  Parser p;
  shared_ptr<Object> ret = p.parse(input);
#ifdef DEBUG_Parser_debug
  p.debug();
#endif
  return ret;
}

shared_ptr<Object> eval_ast(shared_ptr<Object> ast,
                            shared_ptr<Environment> env) {
  switch (ast->type) {
  case SYMBOL: {
    if (not(std::find(keywords.begin(), keywords.end(),
                      to_symbol(ast)->value()) != keywords.end())) {
      shared_ptr<Object> ret = env->get(to_symbol(ast));
      return ret;
    } else
      return ast;
  } break;
  case LIST: {
    shared_ptr<List> ret = list();
    for (auto el : to_list(ast)->elements)
      ret->append(EVAL(el, env));
    return ret;
  } break;
  case VEC: {
    shared_ptr<Vec> ret = vec();
    for (auto el : to_vec(ast)->elements)
      ret->append(EVAL(el, env));
    return ret;
  } break;
  case DICT: {
    shared_ptr<Dict> ret = dict();
    for (auto el : to_dict(ast)->map)
      ret->append(el.first, EVAL(el.second, env));
    return ret;
  } break;
  case EXCEPTION: {
    Runtime::unhandled_exc = to_exception(ast);
    return ast;
  } break;
  default:
    return ast;
  }
}

shared_ptr<Object> quasiquote(shared_ptr<Object> ast) {
  switch (ast->type) {
  case LIST: {
    shared_ptr<List> ast_as_list = to_list(ast);
    if (ast_as_list->elements.size() == 2 and
        ast_as_list->elements[0]->type == SYMBOL and
        to_symbol(ast_as_list->elements[0])->value() == "unquote") {
      return ast_as_list->elements[1];
    } else {
      shared_ptr<List> return_list = list();
      for (int i = ast_as_list->elements.size() - 1; i >= 0; i--) {
        shared_ptr<Object> elt = ast_as_list->elements[i];
        if (elt->type == LIST and to_list(elt)->elements.size() == 2 and
            to_list(elt)->elements[0]->type == SYMBOL and
            to_symbol(to_list(elt)->elements[0])->value() == "splice-unquote") {
          shared_ptr<List> new_return = list();
          new_return->append(symbol("concat"));
          new_return->append(to_list(elt)->elements[1]);
          new_return->append(return_list);
          return_list = new_return;
        } else {
          shared_ptr<List> new_return = list();
          new_return->append(symbol("cons"));
          new_return->append(quasiquote(elt));
          new_return->append(return_list);
          return_list = new_return;
        }
      }
      return return_list;
    }
  } break;
  case VEC: {
    shared_ptr<Vec> ast_as_vec = to_vec(ast);
    shared_ptr<List> return_list = list();
    return_list->append(symbol("vec"));
    for (int i = ast_as_vec->elements.size() - 1; i >= 0; i--) {
      shared_ptr<Object> elt = ast_as_vec->elements[i];
      if (elt->type == LIST and to_list(elt)->elements.size() == 2 and
          to_list(elt)->elements[0]->type == SYMBOL and
          to_symbol(to_list(elt)->elements[0])->value() == "splice-unquote") {
        shared_ptr<List> new_return = list();
        new_return->append(symbol("concat"));
        new_return->append(to_list(elt)->elements[1]);
        new_return->append(return_list);
        return_list = new_return;
      } else {
        shared_ptr<List> new_return = list();
        new_return->append(symbol("cons"));
        new_return->append(quasiquote(elt));
        new_return->append(return_list);
        return_list = new_return;
      }
    }
    return return_list;
  } break;
  case SYMBOL:
  case DICT: {
    shared_ptr<List> return_list = list();
    return_list->append(symbol("quote"));
    return_list->append(ast);
    return return_list;
  } break;
  default:;
    return ast;
  }
}

bool is_macro_call(shared_ptr<Object> ast, shared_ptr<Environment> env) {
  if (ast->type == LIST and to_list(ast)->elements.size() > 1 and
      to_list(ast)->elements[0]->type == SYMBOL) {
    shared_ptr<Symbol> ast_as_symbol = to_symbol(to_list(ast)->elements[0]);
    if (ast_as_symbol->value() == "if" or ast_as_symbol->value() == "fn*" or
        ast_as_symbol->value() == "def!" or ast_as_symbol->value() == "let*" or
        ast_as_symbol->value() == "defmacro!" or
        ast_as_symbol->value() == "expandmacro" or
        ast_as_symbol->value() == "quote" or ast_as_symbol->value() == "do" or
        ast_as_symbol->value() == "quasiquoteexpand" or
        ast_as_symbol->value() == "macroexpand" or
        ast_as_symbol->value() == "quasiquote" or
        ast_as_symbol->value() == "try*" or
        ast_as_symbol->value() == "catch*") {
      return false;
    }
    shared_ptr<Object> f_m = env->get(to_symbol(to_list(ast)->elements[0]));
    if (f_m->type == FUNCTION and to_function(f_m)->is_macro)
      return true;
  }
  return false;
}

shared_ptr<Object> macroexpand(shared_ptr<Object> ast,
                               shared_ptr<Environment> env) {
  while (is_macro_call(ast, env)) {
    cout << "Macro" << endl;
    shared_ptr<Function> mf =
        to_function(env->get(to_symbol(to_list(ast)->elements[0])));
    shared_ptr<List> args = list();
    for (unsigned int i = 1; i < to_list(ast)->elements.size(); i++)
      args->append(to_list(ast)->elements[i]);
    ast = mf->call(args);
  }
  return ast;
}

shared_ptr<Object> EVAL(shared_ptr<Object> input,
                        shared_ptr<Environment> repl_env) {
  while (true) {
    constexpr bool eval_step_by_step = false;
    if constexpr (eval_step_by_step == true) {
      cout << debug_object(input) << endl;
      std::cin.get();
    }

    check_exc();
    input = macroexpand(input, repl_env);
    if (input->type != LIST) {
      shared_ptr<Object> ret;
      ret = eval_ast(input, repl_env);
      check_exc();
      return ret;
    } else {
      if (to_list(input)->elements.empty())
        return input;
      if (to_list(input)->elements[0]->type == SYMBOL) {
        shared_ptr<List> input_as_list = to_list(input);
        shared_ptr<Symbol> first_as_symbol =
            to_symbol(input_as_list->elements[0]);
        if (first_as_symbol->value() == "try*") {
          if (input_as_list->elements.size() == 3 and
              input_as_list->elements[2]->type == LIST and
              to_list(input_as_list->elements[2])->elements[0]->type ==
                  SYMBOL and
              to_list(input_as_list->elements[2])->elements[1]->type ==
                  SYMBOL and
              to_symbol(to_list(input_as_list->elements[2])->elements[0])
                      ->value() == "catch*") {
            catching = true;
            shared_ptr<Object> tried_eval =
                EVAL(input_as_list->elements[1], repl_env);
            if (tried_eval->type != EXCEPTION) {
              catching = false;
              input = tried_eval;
              continue;
            } else {
              repl_env = make_shared<Environment>(repl_env);
              repl_env->set(
                  to_symbol(to_list(input_as_list->elements[2])->elements[1]),
                  tried_eval);
              input = to_list(input_as_list->elements[2])->elements[2];
              catching = false;
              Runtime::unhandled_exc = nil();
              continue;
            }
          } else {
            return Runtime::ret_exception(
                "try*/catch*: syntax error. it must be (try* CODE (catch* "
                "error ERROR_HANDLE_CODE))");
          }
        } else if (first_as_symbol->value() == "fn*") {
          if (input_as_list->elements.size() == 3 and
              input_as_list->elements[1]->type == LIST) {
            bool valid = true;
            for (auto el : to_list(input_as_list->elements[1])->elements)
              if (el->type != SYMBOL) {
                valid = false;
                break;
              }
            if (valid) {
              return func(to_list(input_as_list->elements[1]),
                          input_as_list->elements[2], repl_env, "");
            } else {
              cout << "fn* parameters must be all symbols" << endl;
              exit(1);
              return nil();
            }
          } else if (input_as_list->elements.size() == 3 and
                     input_as_list->elements[1]->type == VEC) {
            bool valid = true;
            shared_ptr<List> largs = list();
            for (auto el : to_vec(input_as_list->elements[1])->elements)
              if (el->type != SYMBOL) {
                valid = false;
                break;
              } else {
                largs->append(el);
              }
            if (valid) {
              return func(largs, input_as_list->elements[2], repl_env, "");
            } else {
              cout << "fn* parameters must be all symbols" << endl;
              exit(1);
              return nil();
            }
          } else {
            cout << "fn* arguments must be a list of the parameters and the "
                    "body"
                 << endl;
            exit(1);
            return to_obj(nil());
          }
        } else if (first_as_symbol->value() == "let*") {
          if (input_as_list->elements.size() == 3) {
            shared_ptr<Environment> new_env =
                std::make_shared<Environment>(repl_env);
            if (input_as_list->elements[1]->type == LIST or
                input_as_list->elements[1]->type == VEC) {
              if (input_as_list->elements[1]->type == LIST) {
                shared_ptr<List> new_list;
                new_list = to_list(input_as_list->elements[1]);
                if (new_list->elements.size() % 2 == 0) {
                  for (unsigned int i = 0; i < new_list->elements.size();
                       i += 2) {
                    if (new_list->elements[i]->type == SYMBOL) {
                      new_env->set(new_list->elements[i],
                                   EVAL(new_list->elements[i + 1], new_env));
                      check_exc();
                    } else {
                      cout << "let*: new key entries must be symbols" << endl;
                      exit(1);
                      return to_obj(nil());
                    }
                  }
                  input = input_as_list->elements[2];
                  repl_env = new_env;
                  continue;
                } else
                  return Runtime::ret_exception(
                      "number of new environment entries myst be fair");
              } else {
                shared_ptr<Vec> new_list;
                new_list = to_vec(input_as_list->elements[1]);
                if (new_list->elements.size() % 2 == 0) {
                  for (unsigned int i = 0; i < new_list->elements.size();
                       i += 2) {
                    if (new_list->elements[i]->type == SYMBOL) {
                      new_env->set(new_list->elements[i],
                                   EVAL(new_list->elements[i + 1], new_env));
                      check_exc();
                    } else {
                      cout << "let*: new key entries must be symbols" << endl;
                      exit(1);
                      return to_obj(nil());
                    }
                  }
                  input = input_as_list->elements[2];
                  repl_env = new_env;
                  continue;
                } else
                  return Runtime::ret_exception(
                      "number of new environment entries myst be fair");
              }
            } else
              return Runtime::ret_exception(
                  "let* need a list or vector as first parameter");
          } else
            return Runtime::ret_exception(
                "let* used with the wrong number of arguments");
        } else if (first_as_symbol->value() == "def!") {
          if (input_as_list->elements.size() == 3) {
            shared_ptr<Object> key = input_as_list->elements[1];
            shared_ptr<Object> value = input_as_list->elements[2];
            if (key->type == SYMBOL) {
              shared_ptr<Object> evalued_value = EVAL(value, repl_env);
              check_exc();
              repl_env->set(key, evalued_value);
              return evalued_value;
            } else {
              cout << "def! accept only symbol as key" << endl;
              exit(1);
              return to_obj(nil());
            }
          } else {
            cout << "def! used with the wrong number of arguments" << endl;
            exit(1);
            return to_obj(nil());
          }
        } else if (first_as_symbol->value() == "defmacro!") {
          if (input_as_list->elements.size() == 3) {
            shared_ptr<Object> key = input_as_list->elements[1];
            shared_ptr<Object> value = input_as_list->elements[2];
            if (key->type == SYMBOL) {
              shared_ptr<Object> evalued_value = EVAL(value, repl_env);
              // check_exc();
              evalued_value->is_macro = true;
              repl_env->set(key, evalued_value);
              return evalued_value;
            } else {
              cout << "def! accept only symbol as key" << endl;
              return Runtime::ret_exception("def! accept only symbol as key");
            }
          } else {
            cout << "def! used with the wrong number of arguments" << endl;
            return Runtime::ret_exception(
                "def! used with the wrong number of arguments");
          }
        } else if (first_as_symbol->value() == "macroexpand") {
          if (input_as_list->elements.size() == 2)
            return macroexpand(input_as_list->elements[1], repl_env);
          else {
            cout << "macroexpand: this function take one paramenter" << endl;
            exit(1);
          }
        } else if (first_as_symbol->value() == "if") {
          if (input_as_list->elements.size() == 3 or
              input_as_list->elements.size() == 4) {
            shared_ptr<Object> if_condition =
                EVAL(input_as_list->elements[1], repl_env);
            check_exc();
            if (not((if_condition->type == BOOL and
                     to_bool(if_condition)->value() == false) or
                    if_condition->type == NIL)) {
              input = EVAL(input_as_list->elements[2], repl_env);
              check_exc();
              continue;
            } else {
              if (input_as_list->elements.size() == 4) {
                input = EVAL(input_as_list->elements[3], repl_env);
                check_exc();
              } else {
                input = nil();
              }
              continue;
            }
          } else {
            cout << "if used with the wrong number of arguments" << endl;
            return to_obj(nil());
          }
        } else if (first_as_symbol->value() == "do") {
          if (input_as_list->elements.size() > 1) {
            shared_ptr<Object> ret = nil();
            for (unsigned int i = 1; i < input_as_list->elements.size() - 1;
                 i++) {
              eval_ast(input_as_list->elements[i], repl_env);
            }
            input = input_as_list->elements[input_as_list->elements.size() - 1];
            continue;
          } else {
            cout << "do expression must be called with a list argument" << endl;
            return to_obj(nil());
          }
        } else if (first_as_symbol->value() == "quote") {
          if (input_as_list->elements.size() == 2) {
            return input_as_list->elements[1];
          } else {
            cout << "quote: accept one argument" << endl;
            return to_obj(nil());
          }
        } else if (first_as_symbol->value() == "quasiquote") {
          if (input_as_list->elements.size() == 2) {
            input = quasiquote(input_as_list->elements[1]);
            continue;
          } else {
            cout << "quasiquote take one parameter" << endl;
            exit(1);
          }
        } else if (first_as_symbol->value() == "quasiquoteexpand") {
          if (input_as_list->elements.size() == 2) {
            return quasiquote(input_as_list->elements[1]);
          } else {
            cout << "quasiquoteexpand take one parameter" << endl;
            exit(1);
          }
        }
      }

      shared_ptr<List> evaluated_input = to_list(eval_ast(input, repl_env));
      shared_ptr<Object> first_item_evaluated = evaluated_input->elements[0];
      switch (first_item_evaluated->type) {
      case FUNCTION: {
        // APPLY /INVOKE
        shared_ptr<Function> f = to_function(first_item_evaluated);
        shared_ptr<List> args = list();
        if (f->compiled) {
          for (unsigned int i = 1; i < evaluated_input->elements.size(); i++) {
            args->append(evaluated_input->elements[i]);
          }
          return f->call(args);
        } else {
          shared_ptr<Environment> closure =
              make_shared<Environment>(f->calling_env);
          if (f->last_is_variadic >= 0) {
            if (evaluated_input->elements.size() > f->last_is_variadic) {
              for (unsigned int i = 0; i < f->last_is_variadic; i++) {
                closure->set(f->arguments->elements[i],
                             evaluated_input->elements[i + 1]);
              }
              shared_ptr<List> varargs = list();
              for (unsigned int i = f->last_is_variadic;
                   i < evaluated_input->elements.size() - 1; i++) {
                varargs->append(evaluated_input->elements[i + 1]);
              }
              closure->set(f->arguments->elements[f->last_is_variadic],
                           varargs);
            } else {
              return to_obj(Runtime::ret_exception(
                  "Funcion <" + f->calling_env->get_key(f->shared_from_this()) +
                  ">: wrong number of parameters"));
            }
          } else {
            if (evaluated_input->elements.size() ==
                f->arguments->elements.size() + 1) {
              for (unsigned int i = 1; i < evaluated_input->elements.size();
                   i++) {
                closure->set(f->arguments->elements[i - 1],
                             evaluated_input->elements[i]);
              }
            } else {
              return to_obj(Runtime::ret_exception(
                  "Funcion <" + f->calling_env->get_key(f->shared_from_this()) +
                  ">: wrong number of parameters"));
            }
          }
          repl_env = closure;
          input = f->expression;
          continue;
        }
      } break;
      default:
        cout << "invoke/apply: evaluating a list not starting with a function "
                "type"
             << endl;
        return input;
      }
    }
  }
}

std::string PRINT(shared_ptr<Object> input) {
  string ret = "";
  ret += print_element(input);
  return ret;
}

} // namespace ml

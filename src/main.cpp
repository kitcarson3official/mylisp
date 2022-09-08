#include "linenoise.hpp"
#include "mylisp.hpp"
#include "parser.hpp"
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  ml::Runtime rnt;
  if (argc == 1) {
    // REPL
    const std::string history_path = "history.txt";
    linenoise::LoadHistory(history_path.c_str());
    std::string cmd;
    while (rnt.running) {
      linenoise::Readline("user> ", cmd);
      std::string ret = rep(cmd, rnt.env());
      if (rnt.message_signal->type == ml::SIGNAL and
          ml::to_signal(rnt.message_signal)->_value == ml::QUIT) {
        break;
      }
      cout << ret << endl;
      linenoise::AddHistory(cmd.c_str());
    }
    linenoise::SaveHistory(history_path.c_str());
    return 0;
  } else {
    // LOAD FILE
    shared_ptr<ml::List> eargv = ml::list();
    ml::Parser p;
    for (unsigned int i = 2; i < argc; i++) {
      eargv->append(p.parse(argv[i]));
    }
    rnt.env()->set(ml::str("ARGV"), eargv);
    cout << ml::rep("(load-file \"" + std::string(argv[1]) + "\")", rnt.env())
         << endl;
  }
  return 0;
}

# mylisp
the lisp interpreter the world was waiting for

My version of LISP in C++ following (not properly) the [Make a Lisp](https://github.com/kanaka/mal) project.

# BUILD
The only dependency is [cpp-linenoise](https://github.com/yhirose/cpp-linenoise)
and you need a c++-14 at least compatible compiler<br>
there are a cmake and a powershell scripts to help you compile, run and clean the
project but it is really simple to compile it in you preferred way.

# INTERPRETER
at the moment the interpreter has the following ***reserved keywords***:
- (fn* (***list of args***) ***body***)
- (def! ***simbol*** ***expr***)
- (let* (**list of symbols and values**) **expr**)
- (if **cond** **expr-true** **expr-false**))
- (do **list of expr**) ; evalue all expressions and return the last one
- (quote **args**)
- (quasiquote **args**)
- (try* A (catch* E B)) ; try to eval A , if any exception occurs it eval B with exception value bind to E symbol

and the following built-in functions: 
<br>
- (quit)
- (eval **ast**)<br/>

  ***basic math***<br/>
- (+ **args**)
- (* **args**)
- (- **args**)
- (/ **args**)<br/>

  ***boolean***<br/>

- (= **arg1** **arg2**)
- (> **arg1** **arg2**)
- (>= **arg1** **arg2**)
- (< **arg1** **arg2**)
- (<= **arg1** **arg2**)
- (not **arg**)<br/>

  ***list***<br/>

- (list **args**)                       ; create a new list
- (list? **arg**)                       ; check the first argument is a list
- (empty? **arg**)                      ; check if the first argument is an empty list
- (count **arg**)                       ; count the elements of the first argument if it's a list<br/>

  ***string***<br/>

- (pr-str **args**)                     ; concat strings with a space, interpret special characters and returs it
- (str **args**)                        ; concat strings without a space, does not interpret special characters and returns it<br/>

  ***print***<br/>

- (prn **args**)                        ; concat with a space interpreting special characters and print to screen and returns nil
- (println **args**)                    ; concat with a space without interpreting special characters and print to screen and returns nil<br/>

  ***evaluation***<br/>

- (read-string **arg**)                 ; eval the first argument (must be a string) and returns it
- (slurp **arg**)                       ; open the first argument (string) as a filename and returns the content as a string
- (load-file **arg**)                   ; eval the content of a file<br/>

  ***atoms***<br/>

- (atom **arg**)                        ; new atom with the value of the paramater
- (atom? **arg**)                       ; return if the parameter is an atom
- (deref **arg**)                       ; return the content of an atom
- (reset! **arg1** **arg2**)            ; set the value of arg1 atom to arg2
- (swap! **arg1** **arg2** **args**)    ; set the value of the arg1 atom to the result of function arg2 called with parameters (arg1 args)
```

---

# REPL AND CLI

there is a repl mode and a cli
the last as the following pattern

mylisp FILENAME [ARGS ...]
```
and eval the content of FILENAME passing ARGS to it

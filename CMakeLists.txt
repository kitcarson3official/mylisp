cmake_minimum_required(VERSION 3.12)
project(mylisp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  mylisp.hpp
  types.cpp   types.hpp
  env.cpp     env.hpp
  parser.cpp  parser.hpp
  printer.cpp printer.hpp
  repl.cpp    repl.hpp
  core.cpp    core.hpp
  extern.cpp  extern.hpp
  )
list(TRANSFORM SOURCES PREPEND "src/")
add_library(libmylisp ${SOURCES})
add_library(libmylispextern
  src/extern.cpp
)
set_target_properties(libmylisp PROPERTIES LINKER_LANGUAGE CXX)

add_executable(mylisp
  src/main.cpp)
target_link_libraries(mylisp libmylisp libmylispextern)

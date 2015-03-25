cmake_minimum_required(VERSION 2.6)

project(viennautils)

try_compile(HAS_VARIADIC_MACRO_SUPPORT
  ${CMAKE_BINARY_DIR}
  $ENV{VIENNAUTILSPATH}/cmake/test_need_variadics.cpp
)

if (HAS_VARIADIC_MACRO_SUPPORT)
  add_definitions(-DBOOST_PP_VARIADICS=1)
else ()
  message(FATAL_ERROR "ViennaUtils needs support for variadic macros, enable them or update your (ancient) compiler")
endif()

# libraries in override take precedence over system libraries
include_directories(BEFORE $ENV{VIENNAUTILSPATH}/override)
include_directories($ENV{VIENNAUTILSPATH}/include)

file(GLOB_RECURSE VIENNAUTILS_DFISE_SRC src/viennautils/dfise/*.cpp)
add_library(viennautils_dfise ${VIENNAUTILS_DFISE_SRC})

add_subdirectory(examples)

try_compile(HAS_VARIADIC_MACRO_SUPPORT ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/test_need_variadics.cpp)

if (HAS_VARIADIC_MACRO_SUPPORT)
  vienna_build_add_definitions(-DBOOST_PP_VARIADICS=1)
else ()
  message(FATAL_ERROR "ViennaUtils needs support for variadic macros, enable them or update your (ancient) compiler")
endif ()

# libraries in override take precedence over system libraries
vienna_build_include_directories(BEFORE override)
vienna_build_include_directories(include)

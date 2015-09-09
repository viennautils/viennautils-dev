vienna_build_add_definitions(-DBOOST_NO_LONG_LONG=1)

# libraries in override take precedence over system libraries
vienna_build_include_directories(BEFORE override)
vienna_build_include_directories(include)

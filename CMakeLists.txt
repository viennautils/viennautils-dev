cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
include (cmake/vienna_build_common.cmake NO_POLICY_SCOPE)

vienna_build_setup_project(viennautils)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

if (VIENNA_BUILD_IS_MAIN_PROJECT)
  option(BUILD_EXAMPLES "Build example programs" OFF)
endif ()

file(GLOB_RECURSE FILESYSTEM_SRC src/viennautils/filesystem/*.cpp)
add_library(viennautils_filesystem ${FILESYSTEM_SRC})

file(GLOB_RECURSE DFISE_SRC src/viennautils/dfise/*.cpp)
add_library(viennautils_dfise ${DFISE_SRC})
target_link_libraries(viennautils_dfise viennautils_filesystem)

if (VIENNA_BUILD_IS_MAIN_PROJECT AND BUILD_EXAMPLES)
  add_subdirectory(examples)
else ()
  add_subdirectory(examples EXCLUDE_FROM_ALL)
endif ()

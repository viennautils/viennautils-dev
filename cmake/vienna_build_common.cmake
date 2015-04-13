if (NOT COMMAND vienna_build_setup_project) #this servers as an include guard

cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0013 NEW)
cmake_policy(SET CMP0014 NEW)

#this function sets various variables before actually calling the include/add_subdirective command
#  some of the variables set here are automatic variables that either have to be emulated (for the include call)
#  or that are not set according to specification (for example PROJECT_SOURCE_DIR should point to the directory that
#  contains the "closest" CMakeLists.txt that has project() command in it - but this just not the case when we use
#  add_subdirectory() for subprojects...)
#make sure that you only invoke it within the context of a new scope (such as a function), otherwise you won't be
#  able to recover the original values
macro (vienna_build_detail_prepare_subproject_variables SUB_PROJECT)
  set(VIENNA_BUILD_IS_MAIN_PROJECT 0)
  set(PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR}/${SUB_PROJECT})
  set(CMAKE_CURRENT_LIST_FILE ${VIENNA_BUILD_${SUB_PROJECT}_SOURCE_DIR}/CMakeLists.txt)
  set(PROJECT_SOURCE_DIR ${VIENNA_BUILD_${SUB_PROJECT}_SOURCE_DIR})
endmacro ()

function (vienna_build_detail_include_prerequisites_and_globals SUB_PROJECT)
  #the scope provided by the function prevents subsequent variable changes from becoming permanent
  vienna_build_detail_prepare_subproject_variables(${SUB_PROJECT})
  include(${PROJECT_SOURCE_DIR}/cmake/prerequisites_and_globals.cmake OPTIONAL)
endfunction ()

function (vienna_build_detail_add_subproject_directory SUB_PROJECT)
  #the scope provided by the function prevents subsequent variable changes from becoming permanent
  vienna_build_detail_prepare_subproject_variables(${SUB_PROJECT})
  add_subdirectory(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
endfunction ()

# ---------------- PUBLIC INTERACE ----------------

function (vienna_build_add_definitions)
  set(DEFINITIONS ${ARGN})
  set(VIENNA_BUILD_GLOBAL_DEFINES ${VIENNA_BUILD_GLOBAL_DEFINES} ${DEFINITIONS} CACHE INTERNAL "")
  if (VIENNA_BUILD_IS_MAIN_PROJECT)
    add_definitions(${DEFINITIONS})
  endif ()
endfunction ()

#this function adds the passed include paths to the corresponding global variable
#  (either VIENNA_BUILD_GLOBAL_BEFORE_INCLUDES or VIENNA_BUILD_GLOBAL_INCLUDES)
#  relative include paths are made absolute in the process
function (vienna_build_include_directories MAYBE_BEFORE)
  string(COMPARE EQUAL "${MAYBE_BEFORE}" "BEFORE" IS_BEFORE)
  if (IS_BEFORE)
    set(ALL_INCLUDE_DIRS ${ARGN})
  else ()
   set(ALL_INCLUDE_DIRS ${MAYBE_BEFORE} ${ARGN})
  endif ()

  foreach (INCLUDE_DIR ${ALL_INCLUDE_DIRS})
    if (IS_ABSOLUTE INCLUDE_DIR)
      set(ABSOLUTE_INCLUDE_DIRS ${ABSOLUTE_INCLUDE_DIRS} ${INCLUDE_DIR})
    else ()
      set(ABSOLUTE_INCLUDE_DIRS ${ABSOLUTE_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/${INCLUDE_DIR})
    endif ()
  endforeach()

  if (IS_BEFORE)
    set(VIENNA_BUILD_GLOBAL_BEFORE_INCLUDES ${VIENNA_BUILD_GLOBAL_BEFORE_INCLUDES} ${ABSOLUTE_INCLUDE_DIRS} CACHE INTERNAL "")
    if (VIENNA_BUILD_IS_MAIN_PROJECT)
      include_directories(BEFORE ${ABSOLUTE_INCLUDE_DIRS})
    endif ()
  else ()
    set(VIENNA_BUILD_GLOBAL_INCLUDES ${VIENNA_BUILD_GLOBAL_INCLUDES} ${ABSOLUTE_INCLUDE_DIRS} CACHE INTERNAL "")
    if (${VIENNA_BUILD_IS_MAIN_PROJECT})
      include_directories(${ABSOLUTE_INCLUDE_DIRS})
    endif ()
  endif ()
endfunction ()

function (vienna_build_setup_project PNAME)
  set(SUB_PROJECTS ${ARGN})

  project (${PNAME})

  #since we are using CACHE INTERNAL variables as global variables and those are stored between CMAKE calls
  #we have to clean them up in the beginning of every new cmake call
  string(COMPARE EQUAL "${PNAME}" "${VIENNA_BUILD_MAIN_PROJECT}" RECOMPILING_MAIN_CMAKE)
  if (RECOMPILING_MAIN_CMAKE OR (NOT DEFINED VIENNA_BUILD_MAIN_PROJECT))
    set(VIENNA_BUILD_IS_MAIN_PROJECT 1)
    set(VIENNA_BUILD_IS_MAIN_PROJECT 1 PARENT_SCOPE)
    set(VIENNA_BUILD_MAIN_PROJECT ${PNAME} CACHE INTERNAL "")
    set(VIENNA_BUILD_SUB_PROJECTS ${SUB_PROJECTS} CACHE INTERNAL "")
    set(VIENNA_BUILD_GLOBAL_DEFINES "" CACHE INTERNAL "")
    set(VIENNA_BUILD_GLOBAL_INCLUDES "" CACHE INTERNAL "")
    set(VIENNA_BUILD_GLOBAL_BEFORE_INCLUDES "" CACHE INTERNAL "")
    
    #setup the VIENNA_BUILD_${SUB_PROJECT}_SOURCE_DIR variables
    foreach (SUB_PROJECT ${SUB_PROJECTS})
      string(TOUPPER ${SUB_PROJECT} SUB_PROJECT_UPPER)
      set(VIENNA_BUILD_${SUB_PROJECT}_SOURCE_DIR $ENV{${SUB_PROJECT_UPPER}PATH} CACHE STRING "path to source directory of sub-project ${SUB_PROJECT}")
    endforeach ()

    #first we include our own prerequisites_and_globals file
    include(${PROJECT_SOURCE_DIR}/cmake/prerequisites_and_globals.cmake OPTIONAL)

    #gather all definitions, includes and before_includes
    foreach (SUB_PROJECT ${SUB_PROJECTS})
      vienna_build_detail_include_prerequisites_and_globals(${SUB_PROJECT})
    endforeach ()

    #now set the global values at the scope of the root CMakeLists
    message(STATUS "global definitions: ${VIENNA_BUILD_GLOBAL_DEFINES}")
    message(STATUS "global before includes: ${VIENNA_BUILD_GLOBAL_BEFORE_INCLUDES}")
    message(STATUS "global normal includes: ${VIENNA_BUILD_GLOBAL_INCLUDES}")
    add_definitions(${VIENNA_BUILD_GLOBAL_DEFINES})
    include_directories(BEFORE ${VIENNA_BUILD_GLOBAL_BEFORE_INCLUDES})
    include_directories(${VIENNA_BUILD_GLOBAL_INCLUDES})
  else ()
    #as a sub-project we only check that our required sub-projects were specified in the main-project
    foreach (SUB_PROJECT ${SUB_PROJECTS})
      if (NOT (";${VIENNA_BUILD_SUB_PROJECTS};" MATCHES ";${SUB_PROJECT};"))
        message(FATAL_ERROR "sub-project ${SUB_PROJECT} required by project ${PNAME} not specified in main-project ${VIENNA_BUILD_MAIN_PROJECT}")
      endif()
    endforeach ()
  endif ()
endfunction ()

function (vienna_build_add_subprojects)
  if (VIENNA_BUILD_IS_MAIN_PROJECT)
    #now proceed with actually adding the sub-directories
    foreach (SUB_PROJECT ${VIENNA_BUILD_SUB_PROJECTS})
      vienna_build_detail_add_subproject_directory(${SUB_PROJECT})
    endforeach ()
  endif ()
endfunction()

endif ()


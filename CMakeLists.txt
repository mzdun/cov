cmake_minimum_required (VERSION 3.25)
project (cov
  DESCRIPTION "Code coverage presented locally"
  VERSION 0.26.2
  LANGUAGES CXX
)

set(PROJECT_VERSION_SHORT "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(PROJECT_VERSION_STABILITY "" # or "-alpha", or "-beta", or "-rc.1", or "-rc.2"
    CACHE STRING "" FORCE)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "Building in source firectory is not supported")
endif()

set(STRINGS
  args
  cov
  errors
  log_format
  cov_checkout
  cov_config
  cov_init
  cov_log
  cov_module
  cov_refs
  cov_report
  cov_reset
  cov_show
  strip_excludes
)
set(LANGUAGES pl en)

set(COV_TESTING ON CACHE BOOL "Compile and/or run self-tests")
set(COV_SANITIZE OFF CACHE BOOL "Compile with sanitizers enabled")
set(COV_CUTDOWN_OS OFF CACHE BOOL "Run tests on cutdown OS (e.g. GitHub docker)")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}/conan" "${PROJECT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_BINARY_DIR}/conan")

set(CONAN_CMAKE_SILENT_OUTPUT ON)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(libgit2 REQUIRED)
find_package(fmt REQUIRED)
find_package(mbits-args REQUIRED)
find_package(mbits-lngs REQUIRED)
find_package(ctre REQUIRED)
find_package(expat REQUIRED)
find_package(libzip REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(lzma_sdk REQUIRED)

find_program(SassC_EXECUTABLE sassc REQUIRED)
message(STATUS "SassC_EXECUTABLE is: ${SassC_EXECUTABLE}")

if (COV_TESTING)
  enable_testing()

  find_package(GTest REQUIRED)
  find_program(JsonRunner_EXECUTABLE json-runner REQUIRED HINTS
    "${PROJECT_SOURCE_DIR}/build/downloads"
    "${PROJECT_SOURCE_DIR}/../json-runner/build/release/bin"
    "${PROJECT_SOURCE_DIR}/../runner/build/release/bin"
  )
  message(STATUS "JsonRunner_EXECUTABLE is: ${JsonRunner_EXECUTABLE}")

  set(TEST_REPORT_DIR "${PROJECT_BINARY_DIR}/test-results")
  set(TEST_SANITIZER_TAG "")
  if (COV_SANITIZE)
    set(TEST_SANITIZER_TAG "-sanitizer")
  endif()
  set(TEST_REPORT_FILE "${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}${TEST_SANITIZER_TAG}.xml")

  if (cov_COVERALLS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    endif()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage -lgcov")
    endif()
  endif()
endif()

include(output_dirs_setup)
old_conan_output_dirs_setup()

if (MSVC)
  set(ADDITIONAL_WALL_FLAGS
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS
    /utf-8
    /permissive-
    /Zc:__cplusplus
    /W4
    /w14242
    /w14254
    /w14263
    /w14265
    /w14287
    /we4289
    /w14296
    /w14311
    /w14545
    /w14546
    /w14547
    /w14549
    /w14555
    /w14619
    /w14640
    /w14826
    /w14905
    /w14906
    /w14928
    /w14946)
  if (COV_SANITIZE)
    list(APPEND ADDITIONAL_WALL_FLAGS
      /fsanitize=address
      /D_DISABLE_VECTOR_ANNOTATION
      /D_DISABLE_STRING_ANNOTATION
    )
  endif()
else()
  set(ADDITIONAL_WALL_FLAGS
      -Wall -Wextra
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Wunused
      -Woverloaded-virtual
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
  )
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND ADDITIONAL_WALL_FLAGS
      -fcolor-diagnostics
      -Wno-braced-scalar-init
    # -Wlifetime
    )
  else()
    list(APPEND ADDITIONAL_WALL_FLAGS
      -fdiagnostics-color
      -Wmisleading-indentation
      -Wduplicated-cond
      -Wduplicated-branches
      -Wlogical-op
      -Wuseless-cast
      )
  endif()
  if (COV_SANITIZE)
    list(APPEND ADDITIONAL_WALL_FLAGS
      -fsanitize=address
      -fsanitize=undefined
      
    )
    list(APPEND ADDITIONAL_LINK_FLAGS
      -fsanitize=address
      -fsanitize=undefined
    )
  endif()
endif()

if (COV_SANITIZE)
  message(STATUS "This build is prepared for sanitization")
endif()

include(${PROJECT_SOURCE_DIR}/packages/config.cmake)
include(CPack)

set(BINARY_DIR bin)
set(ROOT_DIR "..")
set(SHARE_DIR share/cov-${PROJECT_VERSION_SHORT})
set(CORE_DIR libexec/cov)
set(CORE_ROOT_DIR "../..")

message(STATUS "Sanitizer: ${COV_SANITIZE}")

include(cov)

add_subdirectory(data)
add_subdirectory(external)
add_subdirectory(libs)
add_subdirectory(extras)
add_subdirectory(apps)

execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/packages/system.py props
    OUTPUT_VARIABLE COV_PROPERTIES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(TOLOWER ${CMAKE_CXX_COMPILER_ID} COV_PROPERTIES_COMPILER)
if (COV_PROPERTIES_COMPILER STREQUAL "gnu")
  set(COV_PROPERTIES_COMPILER "gcc")
endif()
string(APPEND COV_PROPERTIES "\n-pbuild_type='${CMAKE_BUILD_TYPE}'")
string(APPEND COV_PROPERTIES "\n-pcompiler='${COV_PROPERTIES_COMPILER}'")
if (MSVC)
  string(APPEND COV_PROPERTIES "\n-pcompiler.version='${MSVC_VERSION}'")
  string(APPEND COV_PROPERTIES "\n-pcompiler.toolset='${CMAKE_VS_PLATFORM_TOOLSET}'")
else()
  string(APPEND COV_PROPERTIES "\n-pcompiler.version='${CMAKE_CXX_COMPILER_VERSION}'")
endif()
if ("stdc++" IN_LIST CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
  string(APPEND COV_PROPERTIES "\n-pcompiler.libcxx='stdc++'")
elseif("c++" IN_LIST CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
  string(APPEND COV_PROPERTIES "\n-pcompiler.libcxx='c++'")
endif()
if (COV_SANITIZE)
  string(APPEND COV_PROPERTIES "\n-psanitizer=on")
else()
  string(APPEND COV_PROPERTIES "\n-psanitizer=off")
endif()

if (COV_TESTING)
  set(COVERALLS_PREFIX cov_)
  
  set(cov_COVERALLS_DIRS ${LEGACY_COVERAGE})
  include(${PROJECT_SOURCE_DIR}/tools/coveralls/Coveralls.cmake)
endif()

file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/report_answers.txt" CONTENT "${COV_PROPERTIES}\n")
string(REPLACE ";" "\n    include = " SRC_INCLUDE "${COVERED_CODE}")
string(REPLACE ";" "\n    exclude = " SRC_EXCLUDE "${NOT_COVERED_CODE}")
configure_file("${PROJECT_SOURCE_DIR}/.covcollect.in" "${PROJECT_BINARY_DIR}/.covcollect" @ONLY)

cmake_minimum_required(VERSION 3.0)

if(SP_BUILD_DIR)
  set(EXTERNAL_BUILD_DIR ${SP_BUILD_DIR})
else()
  # avoid "unused variable" warning
  set(BOGUS ${SP_RECURSIVE})
  if(NOT DEFINED SP_RECURSIVE)
    set(SP_RECURSIVE 1)
  endif()
endif()

# ------------------------------------------------------------------------- #
# C++ compiler (before project!)
if(APPLE)
  SET(CMAKE_CXX_COMPILER             "/usr/bin/clang++")
else()
  SET(CMAKE_CXX_COMPILER             "/usr/local/bin/clang++")
endif()

# ------------------------------------------------------------------------- #
# Project
project (streamlib CXX) # (is C++)

# C++ flags (after project!)
set(CMAKE_CXX_FLAGS                "-Wall -Werror -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_COVERAGE       "-O0 -g --coverage -DDEBUG")

# ------------------------------------------------------------------------- #
# Compiler-specific C++11 activation.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
    message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
  endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  message(STATUS "[${PROJECT_NAME}] Your C++ compiler supports C++11.")
  message(STATUS "[${PROJECT_NAME}] CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
else ()
  message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

# ------------------------------------------------------------------------- #
# Flags

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(LINUX "YES")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "coverage")
  set(SP_COVERAGE ON)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(SP_TOP_LEVEL 1)
else()
  set(SP_TOP_LEVEL )
endif()

# ------------------------------------------------------------------------- #
# Global directories

set(streamlib_CMAKEFILES_DIR ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY})

set(streamlib_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(streamlib_BUILD_DIR ${CMAKE_BINARY_DIR}/build)

if(EXTERNAL_BUILD_DIR)
  set(streamlib_OUTPUT_DIR ${EXTERNAL_BUILD_DIR})
else()
  set(streamlib_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out)
endif()

set(streamlib_SRC_DIR ${streamlib_ROOT_DIR}/src)
set(streamlib_INC_DIR ${streamlib_ROOT_DIR}/include)
set(streamlib_SPEC_DIR ${streamlib_ROOT_DIR}/spec)

set(EXECUTABLE_OUTPUT_PATH ${streamlib_OUTPUT_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${streamlib_OUTPUT_DIR}/lib)
set(HEADER_OUTPUT_PATH ${streamlib_OUTPUT_DIR}/include)

set(splib_HEADER_OUTPUT_PATH ${HEADER_OUTPUT_PATH}/sp)
set(streamlib_HEADER_OUTPUT_PATH ${splib_HEADER_OUTPUT_PATH}/stream)
set(streamlib_SPEC_REPORTS_DIR ${streamlib_OUTPUT_DIR}/reports)

set(streamlib_DOCS_OUTPUT_DIR ${streamlib_OUTPUT_DIR}/docs)
set(streamlib_REPORT_DIR ${streamlib_OUTPUT_DIR}/reports)

if(SP_COVERAGE)
  set(sp_COVERAGE_DIR ${streamlib_BUILD_DIR}/coverage)
  set(streamlib_COVERAGE_DIR ${sp_COVERAGE_DIR}/streamlib)
endif()

# ------------------------------------------------------------------------- #
# Allocate output dirs

add_custom_command(
  OUTPUT "${EXECUTABLE_OUTPUT_PATH}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${EXECUTABLE_OUTPUT_PATH}"
  VERBATIM
  )
add_custom_target( stream_executable_dir
  DEPENDS "${EXECUTABLE_OUTPUT_PATH}"
  )

add_custom_command(
  OUTPUT "${LIBRARY_OUTPUT_PATH}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${LIBRARY_OUTPUT_PATH}"
  VERBATIM
  )
add_custom_target( stream_library_dir
  DEPENDS "${LIBRARY_OUTPUT_PATH}"
  )

if(SP_COVERAGE)
  add_custom_command(
    OUTPUT "${streamlib_COVERAGE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${streamlib_COVERAGE_DIR}"
    VERBATIM
    )
  add_custom_target( streamlib_coverage_dir
    DEPENDS "${streamlib_COVERAGE_DIR}"
    )
endif()

# ------------------------------------------------------------------------- #
# splib submodule

if(SP_RECURSIVE)
  add_subdirectory( splib )
else()
endif()

message(STATUS "[${PROJECT_NAME}] (2) CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# ------------------------------------------------------------------------- #
# Modules paths (exported from splib)

if(SP_RECURSIVE)
  set(CMAKE_MODULE_PATH "${streamlib_ROOT_DIR}/splib/cmake")
else()
  # assume we either have an external build dir or splib's out dir has
  # been copied into ours (e.g. on TC)
  set(CMAKE_MODULE_PATH "${streamlib_OUTPUT_DIR}/cmake")
endif()

message(STATUS "Resolved CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# ------------------------------------------------------------------------- #
# Includes
include(sp_publish_headers)
include(sp_add_subproject)
include(sp_add_components)
include(sp_get_commit)
include(sp_get_linux_info)

# ------------------------------------------------------------------------- #
# Bundler

if(NOT SP_RECURSIVE)
  # execute bundler in published 'bundle' dir
  execute_process(
    COMMAND bundle install
    WORKING_DIRECTORY ${streamlib_OUTPUT_DIR}/bundle
    OUTPUT_VARIABLE bundle_OUTPUT
    RESULT_VARIABLE bundle_RESULT
    )

  if(NOT bundle_RESULT STREQUAL "0")
    message(FATAL_ERROR ${bundle_OUTPUT})
  else()
    message(STATUS ${bundle_OUTPUT})
  endif()
endif()

# ------------------------------------------------------------------------- #
# LLVM

if(NOT APPLE)

  if(LINUX)
    sp_get_linux_info(LINUX_DISTRO LINUX_VERSION)

    if(LINUX_DISTRO STREQUAL "Ubuntu")
      if(LINUX_VERSION VERSION_EQUAL "12.04")
        set(CLANG_VERSION "3.4")
      else(LINUX_VERSION VERSION_EQUAL "14.04" OR LINUX_VERSION VERSION_GREATER "14.04")
        set(CLANG_VERSION "3.5.1")
      else()
        message(FATAL_ERROR "Unsupported Ubuntu version: ${LINUX_VERSION}")
      endif()
    else()
      message(FATAL_ERROR "Unsupported Linux distro: ${LINUX_DISTRO}")
    endif()
  endif()
  
  set(LLVM_ROOT_DIR /usr/local/clang-${CLANG_VERSION})
  find_package (LLVM)
  if(NOT LLVM_FOUND)
    message(FATAL_ERROR "Unable to locate LLVM")
  endif()
  include_directories(${LLVM_INCLUDE_DIRS})
endif()

# ------------------------------------------------------------------------- #
# Revision control

sp_get_commit(streamlib_COMMIT
  PROJECT streamlib
  CWD ${CMAKE_CURRENT_SOURCE_DIR}
  )

message(STATUS "[streamlib] Git commit: ${streamlib_COMMIT}")

# ------------------------------------------------------------------------- #
# External libraries

set(streamlib_DEPS_DIR ${streamlib_OUTPUT_DIR}/deps)
set(streamlib_DEPS_BUILD_DIR ${streamlib_BUILD_DIR}/deps)
set(streamlib_DEPS_INCLUDE_DIR "${streamlib_DEPS_DIR}/include")
set(streamlib_DEPS_LIB_DIR "${streamlib_DEPS_DIR}/lib")

# ------------------------------------------------------------------------- #
# Components
sp_add_components(streamlib
  LIBRARY stream
  logging
  type
  protocolerror
  parameterset
  stream
  listener
  instream
  outstream
  iostream
  protocol
  protocolmanager
  )

# ------------------------------------------------------------------------- #
# Library Target
add_library(stream ${streamlib_SOURCES})
add_dependencies(stream stream_library_dir)

if(SP_RECURSIVE)
add_dependencies( stream
  sp
  libevent
  openssl
  )
else()
endif()

add_dependencies( stream
  publish_headers_streamlib
  )

set(streamlib_INCLUDE_PATH
  ${HEADER_OUTPUT_PATH}
  ${streamlib_DEPS_INCLUDE_DIR}
  # openssl, libevent etc.
  ${splib_DEPS_INCLUDE_DIR}
  )

link_directories(
  ${splib_DEPS_DIR}/lib
  ${streamlib_DEPS_DIR}/lib
  )

message(STATUS "streamlib_INCLUDE_PATH: ${streamlib_INCLUDE_PATH}")
target_include_directories(stream PUBLIC
  ${streamlib_INCLUDE_PATH}
  )

# ------------------------------------------------------------------------- #
# GTest

if(SP_RECURSIVE)
  set(gtest_INCLUDE_DIR ${gtest_SOURCE_DIR}/include)
else()
  set(gtest_INCLUDE_DIR ${HEADER_OUTPUT_PATH})
endif()

# ------------------------------------------------------------------------- #
# Test Suite

add_executable(streamlib_spec ${streamlib_SPEC_SOURCES}) # ${streamlib_SOURCES})
add_dependencies(streamlib_spec stream_executable_dir)

add_dependencies(streamlib_spec
  publish_headers_streamlib
  )

target_link_libraries(streamlib_spec stream)

if( SP_RECURSIVE)
  target_link_libraries(streamlib_spec sp gtest gtest_main event)
else()
  target_link_libraries(streamlib_spec ${LIBRARY_OUTPUT_PATH}/libstream.a ${LIBRARY_OUTPUT_PATH}/libsp.a ${LIBRARY_OUTPUT_PATH}/libgtest.a ${LIBRARY_OUTPUT_PATH}/libgtest_main.a)
endif()

if (NOT APPLE)
  target_link_libraries(streamlib_spec rt)
  if (SP_COVERAGE)
    # LLVM comes with a ton of libraries. We really only need libprofile_rt.
    target_link_libraries(streamlib_spec "${LLVM_LIBRARIES}")
  endif()
endif()

if (NOT WIN32)
  target_link_libraries(streamlib_spec pthread)
endif()

target_include_directories(streamlib_spec PUBLIC
  ${streamlib_INCLUDE_PATH}
  ${gtest_INCLUDE_DIR}
  )

# spec execution target
add_custom_target(
  run_streamlib_spec ALL
  DEPENDS "${streamlib_REPORT_DIR}/streamlib.xml"
  )

# this command runs the actual test suite, generating an XSLT-less XML report.
# We've added a bogus OUTPUT file that never gets created, which seems to be
# the most robust way to force this command to always be run.
add_custom_command(
  OUTPUT ${streamlib_REPORT_DIR}/streamlib.xml always_run_this_command
  COMMAND "${CMAKE_COMMAND}" -DBINARY=${EXECUTABLE_OUTPUT_PATH}/streamlib_spec -DOUTPUT=${streamlib_REPORT_DIR}/streamlib.xml -P ${CMAKE_MODULE_PATH}/sp_run_spec.cmake
  DEPENDS streamlib_spec 
  COMMENT "Executing streamlib spec"
  VERBATIM
  )

# if(SP_RECURSIVE)
#   # run specs in order
#   add_dependencies(
#     run_streamlib_spec
#     run_splib_spec
#     )
# endif()

# ------------------------------------------------------------------------- #
# Docs

if(NOT SP_COVERAGE)

  # hardcoded Doxygen binary path (should be built and shipped by splib)
  set(DOXYGEN_EXECUTABLE ${streamlib_DEPS_DIR}/bin/doxygen)

  # Doxygen-specific settings
  set(sp_DOX_COMMIT ${streamlib_COMMIT})
  set(sp_DOX_PROJECT streamlib)
  set(sp_DOX_BRIEF "The SwarmPlanet Streaming C++ Library")
  set(sp_DOX_README ${streamlib_ROOT_DIR}/README.md)

  set(sp_DOX_PATH ${splib_HEADER_OUTPUT_PATH})
  set(sp_DOX_INC_PATH ${HEADER_OUTPUT_PATH})

  if(SP_RECURSIVE)
    # grab conf template from submodule
    set(dox_input ${streamlib_ROOT_DIR}/splib/docs/splib.conf.in)
  else()
    # assume splib has published conf template via artifacts
    set(dox_input ${streamlib_DOCS_OUTPUT_DIR}/splib.conf.in)
  endif()

  configure_file(
    ${dox_input}
    ${streamlib_DOCS_OUTPUT_DIR}/streamlib.conf
    @ONLY
    )

  add_custom_target(
    streamlib_doc
    ALL
    ${DOXYGEN_EXECUTABLE} ${streamlib_DOCS_OUTPUT_DIR}/streamlib.conf
    WORKING_DIRECTORY ${streamlib_DOCS_OUTPUT_DIR}
    COMMENT "[streamlib] Generating API documentation with Doxygen"
    MAIN_DEPENDENCY ${streamlib_DOCS_OUTPUT_DIR}/streamlib.conf
    VERBATIM
    )

  if(SP_RECURSIVE)
    # assume that if splib_doc can be generated, we have everything that's needed, too.
    # (on TC we assume we got everything we need from the splib artifact)
    add_dependencies(
      streamlib_doc
      splib_doc
      )
  endif()
endif()

# ------------------------------------------------------------------------- #
# Coverage

if(SP_COVERAGE)
  # clean up gcda files before spec run to prevent "profiling:invalid arc tag" messages
  # message("Setting up coverage cleanup")
  
  add_custom_target( cleanup_streamlib_coverage
    ALL
    DEPENDS always_cleanup_streamlib_coverage
    )

  # ensure we clean up right before running the spec
  add_dependencies( run_streamlib_spec
    cleanup_streamlib_coverage
    )
  
  # ensure we clean up right before running the spec
  add_dependencies( 
    cleanup_streamlib_coverage
    streamlib_spec
    )

  add_custom_command(
    OUTPUT always_cleanup_streamlib_coverage
    COMMAND "${CMAKE_COMMAND}" -DSTAMP=${EXECUTABLE_OUTPUT_PATH}/streamlib_spec.coverage_cleanup.stamp -DINPUT_STAMPS=${EXECUTABLE_OUTPUT_PATH}/streamlib_spec -DPATTERNS=${streamlib_CMAKEFILES_DIR}/*.gcda:${streamlib_COVERAGE_DIR}/*.gcov -P ${CMAKE_MODULE_PATH}/sp_purge_files.cmake
    COMMENT "Cleaning up streamlib coverage data."
    VERBATIM
    )
  
endif()

#
# Process .gcda and .gcno files as output from compilation and binary execution w/ --coverage enabled.
# 
if(SP_COVERAGE)

  # only generate HTML if we're in the top-level project
  # if(SP_TOP_LEVEL)
  set(_GEN_OUT 1)
  # else()
  #   set(_GEN_OUT 0)
  #endif()
  
  STRING(REPLACE ";" ":" streamlib_COVERAGE_SOURCES_OUT "${streamlib_COVERAGE_SOURCES}")
  add_custom_command(
    TARGET run_streamlib_spec
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -DREPORT=${streamlib_REPORT_DIR}/streamlib_coverage.html -DCWD=${streamlib_COVERAGE_DIR} -DSOURCES=${streamlib_COVERAGE_SOURCES_OUT} -DLLVM_VERSION=${LLVM_VERSION_STRING} -DLLVM_BIN_DIR=${LLVM_ROOT_DIR}/bin -DGENERATE_HTML=${_GEN_OUT} -DSTAMP=${EXECUTABLE_OUTPUT_PATH}/streamlib_spec.coverage.stamp -DINPUT_STAMPS=${EXECUTABLE_OUTPUT_PATH}/streamlib_spec.succeeded.stamp -DPROJECT_NAME=streamlib -DHEADER_OUTPUT_PATH=${streamlib_HEADER_OUTPUT_PATH} -DPARENT_PROJECT=splib -P ${CMAKE_MODULE_PATH}/sp_process_coverage.cmake 
    COMMENT "Processing coverage"
    VERBATIM
    )
endif() # coverage

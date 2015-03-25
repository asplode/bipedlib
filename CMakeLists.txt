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
project (bipedlib CXX) # (is C++)

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

set(bipedlib_CMAKEFILES_DIR ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY})

set(bipedlib_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(bipedlib_BUILD_DIR ${CMAKE_BINARY_DIR}/build)

if(EXTERNAL_BUILD_DIR)
  set(bipedlib_OUTPUT_DIR ${EXTERNAL_BUILD_DIR})
else()
  set(bipedlib_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out)
endif()

set(bipedlib_SRC_DIR ${bipedlib_ROOT_DIR}/src)
set(bipedlib_INC_DIR ${bipedlib_ROOT_DIR}/include)
set(bipedlib_SPEC_DIR ${bipedlib_ROOT_DIR}/spec)

set(EXECUTABLE_OUTPUT_PATH ${bipedlib_OUTPUT_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${bipedlib_OUTPUT_DIR}/lib)
set(HEADER_OUTPUT_PATH ${bipedlib_OUTPUT_DIR}/include)

# set(splib_HEADER_OUTPUT_PATH ${HEADER_OUTPUT_PATH}/sp)
set(bipedlib_HEADER_OUTPUT_PATH ${HEADER_OUTPUT_PATH}/biped)
set(bipedlib_SPEC_REPORTS_DIR ${bipedlib_OUTPUT_DIR}/reports)

set(bipedlib_DOCS_OUTPUT_DIR ${bipedlib_OUTPUT_DIR}/docs)
set(bipedlib_REPORT_DIR ${bipedlib_OUTPUT_DIR}/reports)

if(SP_COVERAGE)
  set(sp_COVERAGE_DIR ${bipedlib_BUILD_DIR}/coverage)
  set(bipedlib_COVERAGE_DIR ${sp_COVERAGE_DIR}/bipedlib)
endif()

# ------------------------------------------------------------------------- #
# Allocate output dirs

add_custom_command(
  OUTPUT "${EXECUTABLE_OUTPUT_PATH}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${EXECUTABLE_OUTPUT_PATH}"
  VERBATIM
  )
add_custom_target( biped_executable_dir
  DEPENDS "${EXECUTABLE_OUTPUT_PATH}"
  )

add_custom_command(
  OUTPUT "${LIBRARY_OUTPUT_PATH}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${LIBRARY_OUTPUT_PATH}"
  VERBATIM
  )
add_custom_target( biped_library_dir
  DEPENDS "${LIBRARY_OUTPUT_PATH}"
  )

if(SP_COVERAGE)
  add_custom_command(
    OUTPUT "${bipedlib_COVERAGE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${bipedlib_COVERAGE_DIR}"
    VERBATIM
    )
  add_custom_target( bipedlib_coverage_dir
    DEPENDS "${bipedlib_COVERAGE_DIR}"
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
  set(CMAKE_MODULE_PATH "${bipedlib_ROOT_DIR}/splib/cmake")
else()
  # assume we either have an external build dir or splib's out dir has
  # been copied into ours (e.g. on TC)
  set(CMAKE_MODULE_PATH "${bipedlib_OUTPUT_DIR}/cmake")
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
    WORKING_DIRECTORY ${bipedlib_OUTPUT_DIR}/bundle
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

sp_get_commit(bipedlib_COMMIT
  PROJECT bipedlib
  CWD ${CMAKE_CURRENT_SOURCE_DIR}
  )

message(STATUS "[bipedlib] Git commit: ${bipedlib_COMMIT}")

# ------------------------------------------------------------------------- #
# External libraries

set(bipedlib_DEPS_DIR ${bipedlib_OUTPUT_DIR}/deps)
set(bipedlib_DEPS_BUILD_DIR ${bipedlib_BUILD_DIR}/deps)
set(bipedlib_DEPS_INCLUDE_DIR "${bipedlib_DEPS_DIR}/include")
set(bipedlib_DEPS_LIB_DIR "${bipedlib_DEPS_DIR}/lib")

IF(APPLE)

  execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE OSX_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  message(STATUS "OSX_SDK_PATH: ${OSX_SDK_PATH}")

  if(NOT OSX_SDK_PATH)
    message(FATAL_ERROR "Could not resolve SDK path on OSX")
  endif()
  
  set(OSX_FRAMEWORK_PATH ${OSX_SDK_PATH}/System/Library/Frameworks)
  set(GLUT_HEADER_PATH ${OSX_FRAMEWORK_PATH}/GLUT.framework/Versions/A/Headers)
  set(OpenGL_HEADER_PATH ${OSX_FRAMEWORK_PATH}/OpenGL.framework/Versions/A/Headers)
  
  find_path(GLUT_INCLUDE_DIR glut.h
    ${GLUT_HEADER_PATH}
    )
  
  find_path(OPENGL_INCLUDE_DIR gl.h
    ${OpenGL_HEADER_PATH}
    ${OPENGL_LIBRARY_DIR}
    )
  
ENDIF (APPLE)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})

# ------------------------------------------------------------------------- #
# Subprojects
sp_add_subproject( ik )
sp_add_components( ik
  LIBRARY biped
  opengl
  jacobian
  linearr2
  linearr3
  linearr4
  main
  mathmisc
  matrixrmn
  node
  rgbimage
  spherical
  tree
  vectorrn
  misc
  )

set(bipedlib_SOURCES
  ${ik_SOURCES}
  )

set(bipedlib_SPEC_SOURCES
  ${ik_SPEC_SOURCES}
  )

#sp_add_components(bipedlib
#  LIBRARY biped
#  )

# ------------------------------------------------------------------------- #
# Library Target
add_library(biped ${bipedlib_SOURCES})
add_dependencies(biped biped_library_dir)

if(SP_RECURSIVE)
add_dependencies( biped
  sp
  libevent
  openssl
  )
else()
endif()

add_dependencies( biped
  publish_headers_ik
  )

set(bipedlib_INCLUDE_PATH
  ${HEADER_OUTPUT_PATH}
  ${bipedlib_DEPS_INCLUDE_DIR}
  # openssl, libevent etc.
  ${splib_DEPS_INCLUDE_DIR}
  )

link_directories(
  ${splib_DEPS_DIR}/lib
  ${bipedlib_DEPS_DIR}/lib
  )

message(STATUS "bipedlib_INCLUDE_PATH: ${bipedlib_INCLUDE_PATH}")
target_include_directories(biped PUBLIC
  ${bipedlib_INCLUDE_PATH}
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

add_executable(bipedlib_spec ${bipedlib_SPEC_SOURCES}) # ${bipedlib_SOURCES})
add_dependencies(bipedlib_spec biped_executable_dir)

add_dependencies(bipedlib_spec
  publish_headers_ik
  )

target_link_libraries(bipedlib_spec biped ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

if( SP_RECURSIVE)
  target_link_libraries(bipedlib_spec sp gtest gtest_main event)
else()
  target_link_libraries(bipedlib_spec ${LIBRARY_OUTPUT_PATH}/libbiped.a ${LIBRARY_OUTPUT_PATH}/libsp.a ${LIBRARY_OUTPUT_PATH}/libgtest.a ${LIBRARY_OUTPUT_PATH}/libgtest_main.a)
endif()

if (NOT APPLE)
  target_link_libraries(bipedlib_spec rt)
  if (SP_COVERAGE)
    # LLVM comes with a ton of libraries. We really only need libprofile_rt.
    target_link_libraries(bipedlib_spec "${LLVM_LIBRARIES}")
  endif()
endif()

if (NOT WIN32)
  target_link_libraries(bipedlib_spec pthread)
endif()

target_include_directories(bipedlib_spec PUBLIC
  ${bipedlib_INCLUDE_PATH}
  ${gtest_INCLUDE_DIR}
  )

# spec execution target
add_custom_target(
  run_bipedlib_spec ALL
  DEPENDS "${bipedlib_REPORT_DIR}/bipedlib.xml"
  )

# this command runs the actual test suite, generating an XSLT-less XML report.
# We've added a bogus OUTPUT file that never gets created, which seems to be
# the most robust way to force this command to always be run.
add_custom_command(
  OUTPUT ${bipedlib_REPORT_DIR}/bipedlib.xml always_run_this_command
  COMMAND "${CMAKE_COMMAND}" -DBINARY=${EXECUTABLE_OUTPUT_PATH}/bipedlib_spec -DOUTPUT=${bipedlib_REPORT_DIR}/bipedlib.xml -P ${CMAKE_MODULE_PATH}/sp_run_spec.cmake
  DEPENDS bipedlib_spec 
  COMMENT "Executing bipedlib spec"
  VERBATIM
  )

# if(SP_RECURSIVE)
#   # run specs in order
#   add_dependencies(
#     run_bipedlib_spec
#     run_splib_spec
#     )
# endif()

# ------------------------------------------------------------------------- #
# Docs

if(NOT SP_COVERAGE)

  # hardcoded Doxygen binary path (should be built and shipped by splib)
  set(DOXYGEN_EXECUTABLE ${bipedlib_DEPS_DIR}/bin/doxygen)

  # Doxygen-specific settings
  set(sp_DOX_COMMIT ${bipedlib_COMMIT})
  set(sp_DOX_PROJECT bipedlib)
  set(sp_DOX_BRIEF "The SwarmPlanet Bipeding C++ Library")
  set(sp_DOX_README ${bipedlib_ROOT_DIR}/README.md)

  set(sp_DOX_PATH ${splib_HEADER_OUTPUT_PATH})
  set(sp_DOX_INC_PATH ${HEADER_OUTPUT_PATH})

  if(SP_RECURSIVE)
    # grab conf template from submodule
    set(dox_input ${bipedlib_ROOT_DIR}/splib/docs/splib.conf.in)
  else()
    # assume splib has published conf template via artifacts
    set(dox_input ${bipedlib_DOCS_OUTPUT_DIR}/splib.conf.in)
  endif()

  configure_file(
    ${dox_input}
    ${bipedlib_DOCS_OUTPUT_DIR}/bipedlib.conf
    @ONLY
    )

  add_custom_target(
    bipedlib_doc
    ALL
    ${DOXYGEN_EXECUTABLE} ${bipedlib_DOCS_OUTPUT_DIR}/bipedlib.conf
    WORKING_DIRECTORY ${bipedlib_DOCS_OUTPUT_DIR}
    COMMENT "[bipedlib] Generating API documentation with Doxygen"
    MAIN_DEPENDENCY ${bipedlib_DOCS_OUTPUT_DIR}/bipedlib.conf
    VERBATIM
    )

  if(SP_RECURSIVE)
    # assume that if splib_doc can be generated, we have everything that's needed, too.
    # (on TC we assume we got everything we need from the splib artifact)
    add_dependencies(
      bipedlib_doc
      splib_doc
      )
  endif()
endif()

# ------------------------------------------------------------------------- #
# Coverage

if(SP_COVERAGE)
  # clean up gcda files before spec run to prevent "profiling:invalid arc tag" messages
  # message("Setting up coverage cleanup")
  
  add_custom_target( cleanup_bipedlib_coverage
    ALL
    DEPENDS always_cleanup_bipedlib_coverage
    )

  # ensure we clean up right before running the spec
  add_dependencies( run_bipedlib_spec
    cleanup_bipedlib_coverage
    )
  
  # ensure we clean up right before running the spec
  add_dependencies( 
    cleanup_bipedlib_coverage
    bipedlib_spec
    )

  add_custom_command(
    OUTPUT always_cleanup_bipedlib_coverage
    COMMAND "${CMAKE_COMMAND}" -DSTAMP=${EXECUTABLE_OUTPUT_PATH}/bipedlib_spec.coverage_cleanup.stamp -DINPUT_STAMPS=${EXECUTABLE_OUTPUT_PATH}/bipedlib_spec -DPATTERNS=${bipedlib_CMAKEFILES_DIR}/*.gcda:${bipedlib_COVERAGE_DIR}/*.gcov -P ${CMAKE_MODULE_PATH}/sp_purge_files.cmake
    COMMENT "Cleaning up bipedlib coverage data."
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
  
  STRING(REPLACE ";" ":" bipedlib_COVERAGE_SOURCES_OUT "${bipedlib_COVERAGE_SOURCES}")
  add_custom_command(
    TARGET run_bipedlib_spec
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -DREPORT=${bipedlib_REPORT_DIR}/bipedlib_coverage.html -DCWD=${bipedlib_COVERAGE_DIR} -DSOURCES=${bipedlib_COVERAGE_SOURCES_OUT} -DLLVM_VERSION=${LLVM_VERSION_STRING} -DLLVM_BIN_DIR=${LLVM_ROOT_DIR}/bin -DGENERATE_HTML=${_GEN_OUT} -DSTAMP=${EXECUTABLE_OUTPUT_PATH}/bipedlib_spec.coverage.stamp -DINPUT_STAMPS=${EXECUTABLE_OUTPUT_PATH}/bipedlib_spec.succeeded.stamp -DPROJECT_NAME=bipedlib -DHEADER_OUTPUT_PATH=${bipedlib_HEADER_OUTPUT_PATH} -DPARENT_PROJECT=splib -P ${CMAKE_MODULE_PATH}/sp_process_coverage.cmake 
    COMMENT "Processing coverage"
    VERBATIM
    )
endif() # coverage

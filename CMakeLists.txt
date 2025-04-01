cmake_minimum_required(VERSION 3.0...3.29)

set(CMAKE_CXX_STANDARD 11)

set(SIMVOLEON_MAJOR_VERSION 2)
set(SIMVOLEON_MINOR_VERSION 1)
set(SIMVOLEON_MICRO_VERSION 0)
set(SIMVOLEON_BETA_VERSION )
set(SIMVOLEON_VERSION ${SIMVOLEON_MAJOR_VERSION}.${SIMVOLEON_MINOR_VERSION}.${SIMVOLEON_MICRO_VERSION}${SIMVOLEON_BETA_VERSION})

project(SIMVoleon VERSION ${SIMVOLEON_MAJOR_VERSION}.${SIMVOLEON_MINOR_VERSION}.${SIMVOLEON_MICRO_VERSION})
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

# ############################################################################
# these will be removed after upgrading CMake minimum version to 3.9.6
set(PROJECT_DESCRIPTION   "A volume rendering library for Coin")
# ############################################################################
 
string(TIMESTAMP SIMVOLEON_BUILD_YEAR "%Y")
math(EXPR SIMVOLEON_SO_VERSION ${PROJECT_VERSION_MAJOR}*20)
set(VERSION ${SIMVOLEON_VERSION})

if(POLICY CMP0072)
  # get rid of OpenGL GLVND warning from CMake 3.11
  cmake_policy(SET CMP0072 NEW)
endif()

if(POLICY CMP0075)
  # get rid of CMAKE_REQUIRED_LIBRARIES warning from CMake 3.12
  cmake_policy(SET CMP0075 NEW)
endif()

# ############################################################################
# Prevent in-source builds, as they often cause severe build problems
# ############################################################################

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "${CMAKE_PROJECT_NAME} requires an out of source build. Please create a separate build directory and run 'cmake <path_to_${CMAKE_PROJECT_NAME}> [options]' from there.")
endif()

# ############################################################################
# Include necessary submodules
# ############################################################################

include(CheckCXXSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# ############################################################################
# Macros & functions
# ############################################################################

function(dump_variable)
  if (OPTION_VERBOSE)
    foreach(f ${ARGN})
      if (DEFINED ${f})
        message("${f} = ${${f}}")
      else()
        message("${f} = ***UNDEF***")
      endif()
    endforeach()
  endif()
endfunction()

function(executable executable_name)
  set(multiValueArgs SOURCES LIBS)
  cmake_parse_arguments(executable "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  add_executable(${executable_name} ${executable_SOURCES})
  target_link_libraries(${executable_name} ${executable_LIBS})
  if (HAVE_ASAN)
    target_compile_options(${executable_name} PUBLIC "-fsanitize=address")
    set_target_properties(${executable_name} PROPERTIES LINK_FLAGS "-fsanitize=address")
  endif()
endfunction()

function(report_prepare)
  set(multiValueArgs IF_APPLE IF_WIN32)
  cmake_parse_arguments(REPORT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  if (REPORT_IF_APPLE AND APPLE)
    list(APPEND res ${REPORT_IF_APPLE})
  endif()
  if (REPORT_IF_WIN32 AND WIN32)
    list(APPEND res ${REPORT_IF_WIN32})
  endif()
  list(APPEND res ${REPORT_UNPARSED_ARGUMENTS})
  list(APPEND PACKAGE_OPTIONS ${res})
  set(PACKAGE_OPTIONS "${PACKAGE_OPTIONS}" PARENT_SCOPE)
endfunction()

# ############################################################################
# Provide options to customise the build
# ############################################################################

option(OPTION_VERBOSE "Verbose build " OFF)
option(SIMVOLEON_BUILD_SHARED_LIBS "Build shared libraries" ON)
option(SIMVOLEON_BUILD_DOCUMENTATION "Build and install API documentation (requires Doxygen)." OFF)
option(SIMVOLEON_BUILD_AWESOME_DOCUMENTATION "Build and install API documentation in new modern style (requires Doxygen)." OFF)
option(SIMVOLEON_BUILD_TESTS "Build test code" OFF)
cmake_dependent_option(SIMVOLEON_BUILD_INTERNAL_DOCUMENTATION "Document internal code not part of the API." OFF "SIMVOLEON_BUILD_DOCUMENTATION" OFF)
cmake_dependent_option(SIMVOLEON_BUILD_DOC_MAN "Build So${Gui} man pages." OFF "SIMVOLEON_BUILD_DOCUMENTATION" OFF)
cmake_dependent_option(SIMVOLEON_BUILD_DOC_QTHELP "Build QtHelp documentation." OFF "SIMVOLEON_BUILD_DOCUMENTATION" OFF)
cmake_dependent_option(SIMVOLEON_BUILD_DOC_CHM "Build compressed HTML help manual (requires HTML help compiler)" OFF "SIMVOLEON_BUILD_DOCUMENTATION" OFF)
option(SIMVOLEON_USE_CPACK "If enabled the cpack subrepo is mandatory" OFF)

report_prepare(
  SIMVOLEON_BUILD_SHARED_LIBS
  SIMVOLEON_BUILD_DOCUMENTATION
  SIMVOLEON_BUILD_AWESOME_DOCUMENTATION
  SIMVOLEON_BUILD_TESTS
  SIMVOLEON_BUILD_INTERNAL_DOCUMENTATION
  SIMVOLEON_BUILD_DOC_MAN
  SIMVOLEON_BUILD_DOC_QTHELP
  SIMVOLEON_BUILD_DOC_CHM
)

# ############################################################################
# Find all necessary and optional dependencies
# ############################################################################

find_package(OpenGL REQUIRED) # FIXME really needed?
find_package(Coin REQUIRED)
if (SIMVOLEON_BUILD_TESTS)
  set(Gui "Qt" CACHE STRING "Target GUI for the example code")
  set(GuiValues "Qt;Xt;Win;Wx" CACHE INTERNAL "List of supported values for the GUI cache variable")
  set_property(CACHE Gui PROPERTY STRINGS ${GuiValues})
  message(STATUS "Example Gui set to '${Gui}'")
  if (Gui STREQUAL "Qt")
    message(WARNING "SoQt GUI binding examples WILL CRASH")
  elseif(Gui STREQUAL "Xt")
  elseif(Gui STREQUAL "Wx")
  elseif(Gui STREQUAL "Win")
    message(WARNING "SoWin GUI binding NOT TESTED")
  else()
    message(FATAL_ERROR "Only Qt,Win, Wx and Xt supported: please set Gui at one of these values")
  endif()
  string(TOUPPER ${Gui} GUI)
  find_package(So${Gui})
  set(EXAMPLE_LINK_LIB So${Gui}::So${Gui})
  find_package(GLUT) # needed only for testcode/tabula/glutclut.c
endif()

# ##########################################################################
# Setup build environment
# ##########################################################################

if(NOT CMAKE_BUILD_TYPE)
  # Has no effect for multi configuration generators (VisualStudio, Xcode).
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose type of build, options are Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif()

check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(memory.h HAVE_MEMORY_H)
if(HAVE_SYS_TYPES_H)
  check_cxx_source_compiles("
    #include <sys/types.h>
    #include <pthread.h>
    int main() { struct timespec timeout; timeout.tv_nsec = 0; return 0; }
  " HAVE_PTHREAD_TIMESPEC_NSEC)
else()
  check_cxx_source_compiles("
    #include <pthread.h>
    int main() { struct timespec timeout; timeout.tv_nsec = 0; return 0; }
  " HAVE_PTHREAD_TIMESPEC_NSEC)
endif()
check_cxx_source_compiles("
  int main() {__builtin_expect (x, 0);return 0;}" HAVE___BUILTIN_EXPECT)

check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files("assert.h;ctype.h;errno.h;float.h;limits.h;locale.h;math.h;setjmp.h;signal.h;stdarg.h;stddef.h;stdio.h;stdlib.h;string.h;time.h" STDC_HEADERS)
# FIXME to be tested if necessary
if(HAVE_WINDOWS_H)
  check_include_files("windows.h;tlhelp32.h" HAVE_TLHELP32_H)
  check_cxx_source_compiles("
    #include <windows.h>
    int main() {
      CreateDirectory(NULL, NULL);
      RemoveDirectory(NULL);
      SetLastError(0);
      GetLastError();
      LocalAlloc(0, 1);
      LocalFree(NULL);
      return 0;
    }
  " HAVE_WIN32_API)
  check_symbol_exists(LoadLibrary windows.h HAVE_WIN32_LOADLIBRARY)
  if(HAVE_WIN32_LOADLIBRARY)
    set(HAVE_DYNAMIC_LINKING 1)
  endif()
  check_symbol_exists(GetEnvironmentVariable windows.h HAVE_GETENVIRONMENTVARIABLE)
endif()
set(USE_EXCEPTIONS ON)

set(PACKAGE ${PROJECT_NAME})
set(PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(PACKAGE_BUGREPORT "coin-support@coin3d.org")
set(PACKAGE_NAME ${PROJECT_NAME})
set(PACKAGE_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(PACKAGE_TARNAME ${PROJECT_NAME_LOWER})
set(PACKAGE_URL "https://github.com/coin3d/${PROJECT_NAME_LOWER}")
set(PACKAGE_VERSION ${PROJECT_VERSION})
set(PACKAGE_HOST ${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_HOST_SYSTEM_NAME})
set(PACKAGE_COMPILER ${CMAKE_CXX_COMPILER})
set(PACKAGE_REQUIREMENTS "Coin, ${PACKAGE_ADDITIONAL_REQUIREMENTS}")

# ############################################################################
# Setup targets in subdirectories
# ############################################################################
add_subdirectory(data)
add_subdirectory(lib)
##### small test programs (to be run interactively)
if (SIMVOLEON_BUILD_TESTS)
  add_subdirectory(testcode)
endif()

############################################################################
# New CPACK section, please see the README file inside cpack.d directory.
if (SIMVOLEON_USE_CPACK)
  add_subdirectory(cpack.d)
endif()

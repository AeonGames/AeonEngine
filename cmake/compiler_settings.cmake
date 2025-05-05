if(CMAKE_VERSION VERSION_LESS "3.12.4")
  add_definitions(-D_USE_MATH_DEFINES)
else()
  add_compile_definitions(_USE_MATH_DEFINES)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(PREPROCESS_ONLY_FLAGS /nologo /P /C /TP)
  set(PREPROCESS_OUTPUT_FLAG "/Fi")
  set(PREPROCESS_VA_OPT ", ##")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /DPROTOBUF_WARNINGS=\"4251 4996\"")
else()
  list(APPEND CMAKE_PREFIX_PATH "C:/msys64/mingw64" "C:/msys64/usr")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE
               PROPERTY STRINGS
                        "Debug"
                        "Release"
                        "MinSizeRel"
                        "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  option(USE_ASAN "Instrument the build to use ASAN sanitizers" OFF)
  option(PROFILING "Instrument the build to generate profiling binaries" OFF)
  if(USE_ASAN)
    # -fsanitize=leak
    set(ASAN_SANITIZERS "-fsanitize=undefined -fsanitize=null -fsanitize=return -fsanitize=address -fsanitize=vptr -fsanitize-address-use-after-scope")
  endif()
  if(PROFILING)
    set(PROFILING_FLAGS "-pg")
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNOMINMAX -Wall -Werror -Wfatal-errors -Wpsabi -O0 -ggdb ${ASAN_SANITIZERS} ${PROFILING_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${ASAN_SANITIZERS} ${PROFILING_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} -DNOMINMAX -Wall -Werror -Wfatal-errors -Wpsabi -O3 -fomit-frame-pointer ${PROFILING_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE
      "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${PROFILING_FLAGS}")
      set(PREPROCESS_ONLY_FLAGS -x c++ -E)
      set(PREPROCESS_OUTPUT_FLAG -o)
      set(PREPROCESS_VA_OPT "__VA_OPT__(,)")
endif()

option(USE_IWYU "Instrument compilation to use include-what-you-use" OFF)
if(USE_IWYU)
  find_program(IWYU_PROGRAM NAMES include-what-you-use iwyu)
  if(IWYU_PROGRAM)
    set(IWYU_COMMAND_LINE ${IWYU_PROGRAM};-Xiwyu;--keep=aeongames/ProtoBufClasses.h CACHE INTERNAL "IWYU Command Line" FORCE)
  endif()
endif()

option(USE_CPPCHECK "Use cppcheck static code analisys" OFF)
find_program(CPPCHECK_PROGRAM NAMES cppcheck)
if(CPPCHECK_PROGRAM AND USE_CPPCHECK)
  set(CMAKE_CXX_CPPCHECK ${CPPCHECK_PROGRAM} --quiet)
endif()

if(NOT ENV{MSYSTEM_PREFIX} STREQUAL "")
  set(PkgConfig_ROOT $ENV{MSYSTEM_PREFIX})
endif()
find_package(PkgConfig)
if(PKG_CONFIG_FOUND AND BASH_EXECUTABLE AND MSYS)
  # In order to properly use pkg-config in a msys2 environment we need to jump
  # through some hoops. CMake seems to call pkg-config without any of the bash
  # environment set up, so we need to set the proper pkg config path inside the
  # cmake process address space.
  execute_process(COMMAND ${BASH_EXECUTABLE} --login -c "echo $PKG_CONFIG_PATH"
                  OUTPUT_VARIABLE PKG_CONFIG_PATH)
  set(ENV{PKG_CONFIG_PATH} ${PKG_CONFIG_PATH})
endif()

option(USE_JEMALLOC "Use jemalloc" OFF)
if(USE_JEMALLOC)
  find_library(JEMALLOC_LIB libjemalloc.a jemalloc)
  link_libraries(${JEMALLOC_LIB})
endif()

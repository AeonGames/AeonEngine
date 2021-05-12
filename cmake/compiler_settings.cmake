if(CMAKE_VERSION VERSION_LESS "3.12.4")
  add_definitions(-D_USE_MATH_DEFINES)
else()
  add_compile_definitions(_USE_MATH_DEFINES)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=null -fsanitize=return -fsanitize=vptr -fsanitize-address-use-after-scope")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=null -fsanitize=return -fsanitize=vptr -fsanitize-address-use-after-scope")
  set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer")
endif()

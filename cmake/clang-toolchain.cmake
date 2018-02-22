# This is a CMAKE_TOOLCHAIN_FILE to use it 
# add -DCMAKE_TOOLCHAIN_FILE=<path to this file>
# to the initial cmake command line call or
# set CMAKE_TOOLCHAIN_FILE to the path of this file
# to the CMAKE GUI.

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)

# CMAKE_TOOLCHAIN_FILE For cross MinGW builds.
# THIS IS NOT REQUIRED FOR NATIVE WINDOWS BUILDS
# to use it add -DCMAKE_TOOLCHAIN_FILE=<path to this file>
# to the initial cmake command line call 
# or set CMAKE_TOOLCHAIN_FILE
# to the path of this file to the CMAKE GUI.



set(CMAKE_SYSTEM_NAME Windows)
# which compilers to use for C and C++
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_SYSROOT /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# mingw-w64-libogg mingw-w64-libvorbis mingw-w64-portaudio mingw-w64-zlib mingw-w64-protobuf
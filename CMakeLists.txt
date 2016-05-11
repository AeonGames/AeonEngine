# Copyright 2016 Rodrigo Jose Hernandez Cordoba
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8.12)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    if(POLICY CMP0020)
        cmake_policy(SET CMP0020 NEW)
    endif(POLICY CMP0020)
endif(COMMAND cmake_policy)

project(AeonEngine)
enable_language(CXX)
enable_language(C)

set(RUNTIME_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/runtime" CACHE PATH "Where to find the Aeon Games Runtime instalation.")
list(APPEND CMAKE_PREFIX_PATH "${RUNTIME_INSTALL_PREFIX}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${RUNTIME_INSTALL_PREFIX}/cmake")
list(APPEND CMAKE_MODULE_PATH "${RUNTIME_INSTALL_PREFIX}/lib/cmake/protobuf")

if(CMAKE_COMPILER_IS_GNUCXX)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    if(COMPILER_SUPPORTS_CXX14)
        list(APPEND CMAKE_CXX_FLAGS "-std=c++14")
    elseif(COMPILER_SUPPORTS_CXX11)
        list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 or C++11 support. Please use a different C++ compiler.")
    endif()
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
endif()

find_package(Vulkan)
find_package(OpenGL)
find_package(Protobuf)

add_subdirectory(engine)
add_subdirectory(application)

#######################################
# Code Formating target
#######################################

find_program (SED_EXECUTABLE sed HINTS ENV ProgramFiles PATH_SUFFIXES Git/usr/bin)
if(WIN32)
    find_program (FIND_EXECUTABLE find HINTS ENV ProgramFiles PATH_SUFFIXES Git/usr/bin NO_SYSTEM_ENVIRONMENT_PATH)
else()
    find_program (FIND_EXECUTABLE find)
endif()
find_program (ASTYLE_EXECUTABLE astyle)

set(ASTYLE_OPTIONS "--options=${CMAKE_SOURCE_DIR}/astylerc")
configure_file("${CMAKE_SOURCE_DIR}/cmake/pre-commit.in" "${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit")

if(WIN32)
    add_custom_target(format-code
        ${ASTYLE_EXECUTABLE} --options=${CMAKE_SOURCE_DIR}/astylerc ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h
        # Look for more useful sed oneliners at http://sed.sourceforge.net/sed1line.txt
        COMMAND
            ${FIND_EXECUTABLE} "\"${CMAKE_SOURCE_DIR}\" -type f -name \"CMakeLists.txt\" -exec \"${SED_EXECUTABLE}\" -i 's/\\t/    /g' {} $<SEMICOLON>"
        COMMAND
            ${FIND_EXECUTABLE} "\"${CMAKE_SOURCE_DIR}\" -type f -name \"CMakeLists.txt\" -exec \"${SED_EXECUTABLE}\" -i 's/[ \\t]*$$//' {} $<SEMICOLON>"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting Code.")
else()
    add_custom_target(format-code
        ${ASTYLE_EXECUTABLE} --options=${CMAKE_SOURCE_DIR}/astylerc ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h
        COMMAND
            ${FIND_EXECUTABLE} ${CMAKE_SOURCE_DIR} -type f -name CMakeLists.txt -exec ${SED_EXECUTABLE} -i 's/\\t/    /g' {} \\$<SEMICOLON>
        COMMAND
            ${FIND_EXECUTABLE} ${CMAKE_SOURCE_DIR} -type f -name CMakeLists.txt -exec ${SED_EXECUTABLE} -i 's/[ \\t]*$$//' {} \\$<SEMICOLON>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting Code.")
endif()

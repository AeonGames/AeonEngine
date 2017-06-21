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
    if(POLICY CMP0053)
        cmake_policy(SET CMP0053 NEW)
    endif(POLICY CMP0053)
endif(COMMAND cmake_policy)

project(AeonEngine)
enable_language(CXX)
enable_language(C)

set(RUNTIME_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/runtime" CACHE PATH "Where to find the Aeon Games Runtime instalation.")
list(APPEND CMAKE_PREFIX_PATH "${RUNTIME_INSTALL_PREFIX}")
list(APPEND CMAKE_LIBRARY_PATH "${RUNTIME_INSTALL_PREFIX}/lib")

list(APPEND CMAKE_MODULE_PATH "${RUNTIME_INSTALL_PREFIX}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
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

find_package(Threads)
find_package(Vulkan)
find_package(OpenGL)
find_package(GLslang)
find_package(Protobuf)
include(FindProtobufExp)
find_package(PythonInterp)
include(FindPythonModule)
if(PYTHONINTERP_FOUND)
    find_python_module(autopep8)
endif()
find_program(XXD_EXECUTABLE NAMES xxd xxd.exe)

find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy.exe)
if(CLANG_TIDY_EXECUTABLE)
    option(USE_CLANG_TIDY "Use clang-tidy (Requires CMake 3.6.0 or Higher)")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "Force compilation database")
    set(CLANG_TIDY_CHECKS "-checks=modernize-*")
    include(AeonEngineFunctions)
endif()

if(CMAKE_HOST_UNIX)
    set(PATH_SEPARATOR :)
elseif(CMAKE_HOST_WIN32)
    set(PATH_SEPARATOR $<SEMICOLON>)
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets)
if(WIN32)
    find_package(Qt5 COMPONENTS WinExtras)
endif(WIN32)
if(Qt5_FOUND)
    add_subdirectory(tools/modelviewer)
endif()

find_program (SED_EXECUTABLE sed HINTS ENV ProgramFiles PATH_SUFFIXES Git/usr/bin)
if(NOT SED_EXECUTABLE)
    message(FATAL_ERROR "Unable to find sed executable.")
endif()
find_program (GREP_EXECUTABLE grep HINTS ENV ProgramFiles PATH_SUFFIXES Git/usr/bin)
if(NOT GREP_EXECUTABLE)
    message(FATAL_ERROR "Unable to find grep executable.")
endif()

add_subdirectory(tools/aeontool)
add_subdirectory(engine)
add_subdirectory(application)

#######################################
# Code Formating target
#######################################

if(WIN32)
    find_program (FIND_EXECUTABLE find HINTS ENV ProgramFiles PATH_SUFFIXES Git/usr/bin NO_SYSTEM_ENVIRONMENT_PATH)
else()
    find_program (FIND_EXECUTABLE find)
endif()

find_program (ASTYLE_EXECUTABLE astyle)

set(ASTYLE_OPTIONS "--options=${CMAKE_SOURCE_DIR}/astylerc")
configure_file("${CMAKE_SOURCE_DIR}/cmake/pre-commit.in" "${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit")

if(MSVC)
    # Look for more useful sed oneliners at http://sed.sourceforge.net/sed1line.txt
    add_custom_target(format-code
        ${ASTYLE_EXECUTABLE} --options=${CMAKE_SOURCE_DIR}/astylerc ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h
        COMMAND
            ${FIND_EXECUTABLE} "\"${CMAKE_SOURCE_DIR}\" -type f -name \"CMakeLists.txt\" -exec \"${SED_EXECUTABLE}\" -i 's/\\t/    /g' {} $<SEMICOLON>"
        COMMAND
            ${FIND_EXECUTABLE} "\"${CMAKE_SOURCE_DIR}\" -type f -name \"CMakeLists.txt\" -exec \"${SED_EXECUTABLE}\" -i 's/[ \\t]*$$//' {} $<SEMICOLON>"
        COMMAND
            ${PYTHON_EXECUTABLE} "-m autopep8 --in-place --aggressive --aggressive -r \"${CMAKE_SOURCE_DIR}/tools/blender/addons/io_mesh_msh\""
        COMMAND
            ${PYTHON_EXECUTABLE} "-m autopep8 --in-place --aggressive --aggressive -r \"${CMAKE_SOURCE_DIR}/tools/blender/addons/io_skeleton_skl\""
        COMMAND
            ${PYTHON_EXECUTABLE} "-m autopep8 --in-place --aggressive --aggressive -r \"${CMAKE_SOURCE_DIR}/tools/blender/addons/io_animation_anm\""
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting Code.")
else()
    find_program (AUTOPEP8_EXECUTABLE autopep8)
    add_custom_target(format-code
        ${ASTYLE_EXECUTABLE} --options=${CMAKE_SOURCE_DIR}/astylerc ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h
        COMMAND
            ${FIND_EXECUTABLE} ${CMAKE_SOURCE_DIR} -type f -name CMakeLists.txt -exec ${SED_EXECUTABLE} -i 's/\\t/    /g' {} \\$<SEMICOLON>
        COMMAND
            ${FIND_EXECUTABLE} ${CMAKE_SOURCE_DIR} -type f -name CMakeLists.txt -exec ${SED_EXECUTABLE} -i 's/[ \\t]*$$//' {} \\$<SEMICOLON>
        COMMAND
            ${AUTOPEP8_EXECUTABLE} --in-place --aggressive --aggressive -r \"${CMAKE_SOURCE_DIR}/tools/blender/addons/io_mesh_msh\"
        COMMAND
            ${AUTOPEP8_EXECUTABLE} --in-place --aggressive --aggressive -r \"${CMAKE_SOURCE_DIR}/tools/blender/addons/io_skeleton_skl\"
        COMMAND
            ${AUTOPEP8_EXECUTABLE} --in-place --aggressive --aggressive -r \"${CMAKE_SOURCE_DIR}/tools/blender/addons/io_animation_anm\"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting Code.")
endif()

###########################################
# Protobuf Python source generation target
###########################################

add_custom_target(generate-python-protobuf-source
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/mesh.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/skeleton.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/animation.proto        
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/vector2.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/vector3.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/vector4.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/quaternion.proto
    SOURCES
        ${CMAKE_SOURCE_DIR}/proto/mesh.proto
        ${CMAKE_SOURCE_DIR}/proto/skeleton.proto
        ${CMAKE_SOURCE_DIR}/proto/animation.proto
        ${CMAKE_SOURCE_DIR}/proto/vector2.proto
        ${CMAKE_SOURCE_DIR}/proto/vector3.proto
        ${CMAKE_SOURCE_DIR}/proto/vector4.proto
        ${CMAKE_SOURCE_DIR}/proto/quaternion.proto
    COMMENT "Generating Protocol Buffers Python Code")

###########################################
# Binary asset generation target.
###########################################
find_program (GRAPHICSMAGICK_EXECUTABLE gm HINTS ENV ProgramFiles PATH_SUFFIXES "GraphicsMagick-1.3.25-Q16")
add_custom_target(generate-binary-assets
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=${RUNTIME_INSTALL_PREFIX}/bin${PATH_SEPARATOR}$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/meshes/suzanne.txt -o ${CMAKE_SOURCE_DIR}/game/meshes/suzanne.msh
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=${RUNTIME_INSTALL_PREFIX}/bin${PATH_SEPARATOR}$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/meshes/cube.txt -o ${CMAKE_SOURCE_DIR}/game/meshes/cube.msh
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=${RUNTIME_INSTALL_PREFIX}/bin${PATH_SEPARATOR}$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/shaders/simple_phong.txt -o ${CMAKE_SOURCE_DIR}/game/shaders/simple_phong.prg
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=${RUNTIME_INSTALL_PREFIX}/bin${PATH_SEPARATOR}$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/shaders/fixed_phong.txt -o ${CMAKE_SOURCE_DIR}/game/shaders/fixed_phong.prg
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=${RUNTIME_INSTALL_PREFIX}/bin${PATH_SEPARATOR}$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/shaders/plain_red.txt -o ${CMAKE_SOURCE_DIR}/game/shaders/plain_red.prg
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=${RUNTIME_INSTALL_PREFIX}/bin ${GRAPHICSMAGICK_EXECUTABLE} convert ${CMAKE_SOURCE_DIR}/game/images/orange.svg ${CMAKE_SOURCE_DIR}/game/images/orange.png
    DEPENDS aeontool ProtoBufClasses
    SOURCES
        ${CMAKE_SOURCE_DIR}/game/meshes/suzanne.txt
        ${CMAKE_SOURCE_DIR}/game/meshes/cube.txt
        ${CMAKE_SOURCE_DIR}/game/shaders/simple_phong.txt
        ${CMAKE_SOURCE_DIR}/game/shaders/fixed_phong.txt
        ${CMAKE_SOURCE_DIR}/game/shaders/plain_red.txt
    COMMENT "Generating binary game assets")

###########################################
# Blender Execution target
###########################################
find_program (BLENDER_EXECUTABLE blender HINTS ENV ProgramFiles PATH_SUFFIXES "Blender Foundation/Blender")
if(BLENDER_EXECUTABLE)
# Right now this is just a shortcut to running Blender,
# it was supposed to set everything up so exporters
# were registered and ready to run
# without making changes to the Blender configuration,
# but that idea didn't work out in the end.
# The target is still useful and I may expand on it later on,
# so it stays for now.
add_custom_target(run-blender
    COMMAND
        "${BLENDER_EXECUTABLE}"
    DEPENDS
        generate-python-protobuf-source
    SOURCES
        ${CMAKE_SOURCE_DIR}/tools/blender/addons/io_mesh_msh/export.py
    COMMENT "Running Blender")
endif()

###########################################
# Clean Build Directory Target
###########################################
if(FIND_EXECUTABLE)
add_custom_target(clean-build
    COMMAND
        ${FIND_EXECUTABLE} '${CMAKE_BINARY_DIR}' ! -name 'CMakeCache.txt' ! -wholename '${CMAKE_BINARY_DIR}' -exec rm -rf {} +
    COMMENT "Cleaning Build you will need to rerun cmake configure and generate")
endif()

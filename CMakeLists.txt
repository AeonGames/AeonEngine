# Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba
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
find_package(Portaudio)
find_package(OggVorbis)
include(AeonEngineFunctions)
include(FindProtobufExp)
find_package(PythonInterp 3)
include(FindPythonModule)
if(PYTHONINTERP_FOUND)
    find_python_module(autopep8)
endif()

find_program(GLOBAL_EXECUTABLE NAMES global global.exe)

find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy.exe)
if(CLANG_TIDY_EXECUTABLE)
    option(USE_CLANG_TIDY "Use clang-tidy (Requires CMake 3.6.0 or Higher)")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "Force compilation database")
    set(CLANG_TIDY_CHECKS "-checks=modernize-*")
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

find_program (SED_EXECUTABLE sed HINTS ENV MINGW_PREFIX MSYS2_PATH ProgramFiles PATH_SUFFIXES bin usr/bin Git/usr/bin)
if(NOT SED_EXECUTABLE)
    message(FATAL_ERROR "Unable to find sed executable.")
endif()
find_program (GREP_EXECUTABLE grep HINTS ENV MINGW_PREFIX MSYS2_PATH ProgramFiles PATH_SUFFIXES usr/bin Git/usr/bin)
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
        COMMAND
            ${PYTHON_EXECUTABLE} "-m autopep8 --in-place --aggressive --aggressive -r \"${CMAKE_SOURCE_DIR}/tools/blender/addons/io_model_mdl\""
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting Code.")
else()
    find_program (AUTOPEP8_EXECUTABLE autopep8)
    add_custom_target(format-code
        ${ASTYLE_EXECUTABLE} --options=${CMAKE_SOURCE_DIR}/astylerc \"${CMAKE_SOURCE_DIR}/*.cpp\" \"${CMAKE_SOURCE_DIR}/*.h\"
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
        COMMAND
            ${AUTOPEP8_EXECUTABLE} --in-place --aggressive --aggressive -r \"${CMAKE_SOURCE_DIR}/tools/blender/addons/io_model_mdl\"
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
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/model.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/pipeline.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/material.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/property.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/vector2.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/vector3.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/vector4.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/quaternion.proto
    COMMAND
        ${PROTOBUF_PROTOC_EXECUTABLE} --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules --proto_path ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/proto/geometry.proto
    SOURCES
        ${CMAKE_SOURCE_DIR}/proto/mesh.proto
        ${CMAKE_SOURCE_DIR}/proto/skeleton.proto
        ${CMAKE_SOURCE_DIR}/proto/animation.proto
        ${CMAKE_SOURCE_DIR}/proto/model.proto
        ${CMAKE_SOURCE_DIR}/proto/pipeline.proto
        ${CMAKE_SOURCE_DIR}/proto/material.proto
        ${CMAKE_SOURCE_DIR}/proto/property.proto
        ${CMAKE_SOURCE_DIR}/proto/vector2.proto
        ${CMAKE_SOURCE_DIR}/proto/vector3.proto
        ${CMAKE_SOURCE_DIR}/proto/vector4.proto
        ${CMAKE_SOURCE_DIR}/proto/quaternion.proto
        ${CMAKE_SOURCE_DIR}/proto/geometry.proto
    COMMENT "Generating Protocol Buffers Python Code")

###########################################
# Binary asset generation target.
###########################################
add_custom_target(generate-binary-assets
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/meshes/suzanne.txt -o ${CMAKE_SOURCE_DIR}/game/meshes/suzanne.msh
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/meshes/cube.txt -o ${CMAKE_SOURCE_DIR}/game/meshes/cube.msh
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/shaders/simple_phong.txt -o ${CMAKE_SOURCE_DIR}/game/shaders/simple_phong.prg
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/shaders/fixed_phong.txt -o ${CMAKE_SOURCE_DIR}/game/shaders/fixed_phong.prg
    COMMAND
        ${CMAKE_COMMAND} -E env PATH=$<TARGET_FILE_DIR:ProtoBufClasses> $<TARGET_FILE:aeontool> -i ${CMAKE_SOURCE_DIR}/game/shaders/plain_red.txt -o ${CMAKE_SOURCE_DIR}/game/shaders/plain_red.prg
    COMMAND
        ${CMAKE_COMMAND} -DCMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/GenerateDevelopmentTextures.cmake
    DEPENDS aeontool ProtoBufClasses
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    SOURCES
        ${CMAKE_SOURCE_DIR}/game/meshes/suzanne.txt
        ${CMAKE_SOURCE_DIR}/game/meshes/cube.txt
        ${CMAKE_SOURCE_DIR}/game/shaders/simple_phong.txt
        ${CMAKE_SOURCE_DIR}/game/shaders/fixed_phong.txt
        ${CMAKE_SOURCE_DIR}/game/shaders/plain_red.txt
        ${CMAKE_SOURCE_DIR}/game/images/development.svg
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
        ${CMAKE_SOURCE_DIR}/tools/blender/addons/io_skeleton_skl/export.py
        ${CMAKE_SOURCE_DIR}/tools/blender/addons/io_animation_anm/export.py
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

###########################################
# Generate VS Code Environment
###########################################
if(CMAKE_GENERATOR MATCHES "MSYS Makefiles")
    # TODO SOURCE_DIRECTORIES could be set and used to call add_subdirectory directly from this top level
    set(SOURCE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}
    application
    application/windows
    engine
    engine/images
    engine/images/png
    engine/renderers/opengl
    engine/renderers/vulkan
    engine/sound/oggvorbis
    engine/sound/portaudio
    tools/aeontool
    tools/modelviewer
    )
    set(INCLIST "")
    foreach(sourcedir ${SOURCE_DIRECTORIES})
        get_property(dirs DIRECTORY ${sourcedir} PROPERTY INCLUDE_DIRECTORIES)
        foreach(dir ${dirs})
            get_filename_component(absolute_dir "${dir}" REALPATH)
            list(APPEND INCLIST ${absolute_dir})
        endforeach()
    endforeach()

    execute_process(COMMAND ${CMAKE_COMMAND} -E env MSYSTEM=MINGW64 ${BASH_EXECUTABLE} --login -i -lc 'g++ -E -x c++ - -v < /dev/null' ERROR_VARIABLE GXXINC OUTPUT_QUIET)
    string(REPLACE "\n" "<newline>" GXXINC ${GXXINC})
    string(REGEX REPLACE ".*#include <\.\.\.> search starts here:<newline>(.*)<newline>End of search list.*" "\"\\1\"" GXXINC ${GXXINC})
    string(REGEX REPLACE "[\t ]+" "" GXXINC ${GXXINC})
    string(REPLACE "<newline>" "\",\n\"" GXXINC ${GXXINC})
    #get_filename_component(GXXINC ${GXXINC} REALPATH)

    list(REMOVE_DUPLICATES INCLIST)
    list(SORT INCLIST)
    set(INCLIST "\"${INCLIST}\"")
    string(REPLACE ";" "\",\n\"" INCLIST "${INCLIST}")

    find_program (BASH_EXECUTABLE bash HINTS ENV MINGW_PREFIX MSYS2_PATH)
    find_program (GDB_EXECUTABLE gdb HINTS ENV MINGW_PREFIX MSYS2_PATH)
    configure_file("${CMAKE_SOURCE_DIR}/cmake/tasks.json.in" "${CMAKE_SOURCE_DIR}/.vscode/tasks.json")
    configure_file("${CMAKE_SOURCE_DIR}/cmake/settings.json.in" "${CMAKE_SOURCE_DIR}/.vscode/settings.json")
    configure_file("${CMAKE_SOURCE_DIR}/cmake/launch.json.in" "${CMAKE_SOURCE_DIR}/.vscode/launch.json")
    configure_file("${CMAKE_SOURCE_DIR}/cmake/c_cpp_properties.json.in" "${CMAKE_SOURCE_DIR}/.vscode/c_cpp_properties.json")
endif()

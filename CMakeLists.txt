# Copyright (C) 2016-2020 Rodrigo Jose Hernandez Cordoba
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

cmake_minimum_required(VERSION 3.10.0)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0072 NEW)
  if(POLICY CMP0020)
    cmake_policy(SET CMP0020 NEW)
  endif(POLICY CMP0020)
  if(POLICY CMP0053)
    cmake_policy(SET CMP0053 NEW)
  endif(POLICY CMP0053)
endif(COMMAND cmake_policy)

project(AeonEngine)
enable_testing()
enable_language(CXX)
enable_language(C)

define_property(GLOBAL PROPERTY PLUGINS INHERITED
  BRIEF_DOCS "Built Plugin List"
  FULL_DOCS "Plugin Targets should add an entry to this list in order to be included into the configuration file.")

define_property(GLOBAL PROPERTY SHARED_LIBRARIES INHERITED
  BRIEF_DOCS "List of shared library targets"
  FULL_DOCS "List of shared library targets")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")

include(compiler_settings)

set(PROXY "" CACHE STRING "Proxy server string for downloads.")
if(PROXY)
  set(ENV{http_proxy} "${PROXY}")
  set(ENV{https_proxy} "${PROXY}")
  set(ENV{ftp_proxy} "${PROXY}")
endif()

set(Portaudio_FIND_QUIETLY ON)

find_program(XXD_EXECUTABLE xxd
             HINTS ENV
                   MINGW_PREFIX
                   MSYS2_PATH
                   ChocolateyInstall)
find_program(BASH_EXECUTABLE bash HINTS ENV MINGW_PREFIX MSYS2_PATH)
find_program(GLOBAL_EXECUTABLE
             NAMES global global.exe
             HINTS ENV MINGW_PREFIX MSYS2_PATH)
find_program(CLANG_EXECUTABLE
             NAMES clang clang.exe
             HINTS ENV MINGW_PREFIX MSYS2_PATH)
find_program(CLANG_TIDY_EXECUTABLE
             NAMES clang-tidy clang-tidy.exe
             HINTS ENV MINGW_PREFIX MSYS2_PATH)

find_package(Git)

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

find_package(ZLIB)
find_package(Threads)
find_package(Protobuf)
find_package(Portaudio)
find_package(OggVorbis)
include(FindProtobufExp)
include(FindPythonModule)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
find_package (Python3 COMPONENTS Interpreter)
find_package(ImageMagick COMPONENTS convert)

get_filename_component(PYTHON_PATH "${Python3_EXECUTABLE}" DIRECTORY)
find_program(AUTOPEP8_EXECUTABLE autopep8
             HINTS ENV
                   MINGW_PREFIX
                   MSYS2_PATH
                   "${PYTHON_PATH}/Scripts")
find_program(CMAKE_FORMAT_EXECUTABLE cmake-format
             HINTS ENV MINGW_PREFIX MSYS2_PATH)

option(BUILD_OPENGL_RENDERER "Build the OpenGL renderer" ON)
option(BUILD_VULKAN_RENDERER "Build the Vulkan renderer" ON)
option(USE_AEONGUI "Use the AeonGUI library for the user interface" OFF)

if(CLANG_TIDY_EXECUTABLE)
  option(USE_CLANG_TIDY "Use of clang-tidy (Requires CMake 3.6.0 or Higher)"
         OFF)
  set(CLANG_TIDY_CHECKS
      "-warnings-as-errors=performance-*,modernize-*,-modernize-pass-by-value"
      "-checks=performance-*,modernize-*,-modernize-pass-by-value")
endif()

if(CMAKE_HOST_UNIX)
  set(PATH_SEPARATOR :)
elseif(CMAKE_HOST_WIN32)
  set(PATH_SEPARATOR $<SEMICOLON>)
endif()

find_program(SED_EXECUTABLE sed
            HINTS
                  "C:\\msys64"
            ENV
                  MINGW_PREFIX
                  MSYS2_PATH
                  ProgramFiles
            PATH_SUFFIXES
                  bin
                  usr/bin
                  Git/usr/bin
)

if(NOT SED_EXECUTABLE)
  message(FATAL_ERROR "Unable to find sed executable.")
endif()
find_program(GREP_EXECUTABLE grep
            HINTS
                  "C:\\msys64"
            ENV
                  MINGW_PREFIX
                  MSYS2_PATH
                  ProgramFiles
            PATH_SUFFIXES
                  usr/bin
                  Git/usr/bin
)
if(NOT GREP_EXECUTABLE)
  message(FATAL_ERROR "Unable to find grep executable.")
endif()

find_program(DOXYGEN_EXECUTABLE doxygen
             HINTS ENV
                   MINGW_PREFIX
                   MSYS2_PATH
                   ProgramFiles
                   ChocolateyInstall
                   ProgramData
             PATH_SUFFIXES /usr/bin chocolatey/bin)
if(DOXYGEN_EXECUTABLE)
  find_program(DOT_EXECUTABLE dot
               HINTS ENV
                     MINGW_PREFIX
                     MSYS2_PATH
                     ProgramFiles
                     ChocolateyInstall
                     ProgramData
               PATH_SUFFIXES /usr/bin chocolatey/bin)
  if(DOT_EXECUTABLE)
    set(HAS_DOT "YES")
  else()
    set(HAS_DOT "NO")
  endif()
  if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    OUTPUT_VARIABLE GIT_HASH)
  else()
    set(GIT_HASH "Unknown Build")
  endif()
  file(RELATIVE_PATH BIN_TO_SRC ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})
  configure_file("${CMAKE_SOURCE_DIR}/cmake/Doxyfile.in" "Doxyfile" @ONLY)
  add_custom_target(generate-documentation
                    COMMAND ${CMAKE_COMMAND} -E remove_directory
                            ${CMAKE_SOURCE_DIR}/docs/static/documentation
                    COMMAND ${DOXYGEN_EXECUTABLE}
                    COMMENT "Generating Documentation")
endif()

add_subdirectory(tools/aeontool)
add_subdirectory(engine)
add_subdirectory(application)
add_subdirectory(tools/worldeditor)

find_path(GTEST_SOURCE_PATH googletest PATHS /usr/src/googletest)
if(GTEST_SOURCE_PATH)
  add_subdirectory(${GTEST_SOURCE_PATH} ${CMAKE_BINARY_DIR}/googletest)
  set(GTEST_LIBRARY gtest)
  set(GMOCK_LIBRARY gmock)
  add_subdirectory(tests)
else()
  find_package(GTest)
  if(GTest_FOUND)
    find_library(GMOCK_LIBRARY
                 NAMES gmock libgmock libgmock.a
                 PATHS "${GTEST_DIR}"
                 PATH_SUFFIXES lib)
    find_library(GMOCK_LIBRARY_DEBUG
                 NAMES gmockd libgmockd libgmockd.a
                 PATHS "${GTEST_DIR}"
                 PATH_SUFFIXES lib)
    add_subdirectory(tests)
  endif()
endif()

#
# Code Formating on pre-commit hook
#

if(WIN32)
  find_program(ASTYLE_EXECUTABLE astyle
               HINTS ENV ProgramData
               PATH_SUFFIXES /usr/bin chocolatey/bin)
  find_program(FIND_EXECUTABLE find
               HINTS ENV ProgramFiles
               PATH_SUFFIXES /usr/bin Git/usr/bin)
else()
  find_program(FIND_EXECUTABLE find)
  find_program(ASTYLE_EXECUTABLE astyle)
endif()

set(ASTYLE_OPTIONS "--options=${CMAKE_SOURCE_DIR}/astylerc")
configure_file("${CMAKE_SOURCE_DIR}/cmake/pre-commit.in"
               "${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit")

#
# Protobuf Python source generation target
#

add_custom_target(
  generate-python-protobuf-source
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/mesh.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/skeleton.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/animation.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/model.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/pipeline.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/material.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/property.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/sampler.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/vector2.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/vector3.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/vector4.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/quaternion.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/geometry.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/transform.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/scene.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/reference.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/framebuffer.proto
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --python_out ${CMAKE_SOURCE_DIR}/tools/blender/modules
          --proto_path ${CMAKE_SOURCE_DIR}/proto
                       ${CMAKE_SOURCE_DIR}/proto/matrix4x4.proto
  SOURCES ${CMAKE_SOURCE_DIR}/proto/mesh.proto
          ${CMAKE_SOURCE_DIR}/proto/skeleton.proto
          ${CMAKE_SOURCE_DIR}/proto/animation.proto
          ${CMAKE_SOURCE_DIR}/proto/model.proto
          ${CMAKE_SOURCE_DIR}/proto/pipeline.proto
          ${CMAKE_SOURCE_DIR}/proto/material.proto
          ${CMAKE_SOURCE_DIR}/proto/property.proto
          ${CMAKE_SOURCE_DIR}/proto/sampler.proto
          ${CMAKE_SOURCE_DIR}/proto/vector2.proto
          ${CMAKE_SOURCE_DIR}/proto/vector3.proto
          ${CMAKE_SOURCE_DIR}/proto/vector4.proto
          ${CMAKE_SOURCE_DIR}/proto/quaternion.proto
          ${CMAKE_SOURCE_DIR}/proto/geometry.proto
          ${CMAKE_SOURCE_DIR}/proto/transform.proto
          ${CMAKE_SOURCE_DIR}/proto/scene.proto
          ${CMAKE_SOURCE_DIR}/proto/reference.proto
          ${CMAKE_SOURCE_DIR}/proto/framebuffer.proto
          ${CMAKE_SOURCE_DIR}/proto/matrix4x4.proto
  COMMENT "Generating Protocol Buffers Python Code")

#
# Binary asset generation target.
#
add_custom_target(
  binary-assets
  COMMAND
          $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_SOURCE_DIR}/game/meshes/suzanne.txt -o
          ${CMAKE_SOURCE_DIR}/game/meshes/suzanne.msh
  COMMAND
          $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_SOURCE_DIR}/game/meshes/cube.txt -o
          ${CMAKE_SOURCE_DIR}/game/meshes/cube.msh
  COMMAND
          $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_SOURCE_DIR}/game/shaders/simple_phong.txt -o
          ${CMAKE_SOURCE_DIR}/game/shaders/simple_phong.prg
  COMMAND
          $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_SOURCE_DIR}/game/shaders/fixed_phong.txt -o
          ${CMAKE_SOURCE_DIR}/game/shaders/fixed_phong.prg
  COMMAND
          $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_SOURCE_DIR}/game/shaders/plain_red.txt -o
          ${CMAKE_SOURCE_DIR}/game/shaders/plain_red.prg
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} -P
          ${CMAKE_SOURCE_DIR}/cmake/GenerateDevelopmentTextures.cmake
  DEPENDS aeontool ProtoBufClasses
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  SOURCES ${CMAKE_SOURCE_DIR}/game/meshes/suzanne.txt
          ${CMAKE_SOURCE_DIR}/game/meshes/cube.txt
          ${CMAKE_SOURCE_DIR}/game/shaders/simple_phong.txt
          ${CMAKE_SOURCE_DIR}/game/shaders/fixed_phong.txt
          ${CMAKE_SOURCE_DIR}/game/shaders/plain_red.txt
          ${CMAKE_SOURCE_DIR}/game/images/development.svg.in
  COMMENT "Generating binary game assets")

#
# Blender Execution target
#
find_program(BLENDER_EXECUTABLE
             NAMES blender blender.exe
             HINTS ENV ProgramFiles
             PATH_SUFFIXES "Blender Foundation/Blender")
if(BLENDER_EXECUTABLE)
  get_filename_component(BLENDER_PATH "${BLENDER_EXECUTABLE}" DIRECTORY)
  execute_process(COMMAND "${BLENDER_EXECUTABLE}" --version
                  OUTPUT_VARIABLE BLENDER_VERSION)
  string(REGEX MATCH
               "Blender [^ \\t]+"
               BLENDER_VERSION
               "${BLENDER_VERSION}")
  string(REGEX
         REPLACE "Blender ([^ ]+)"
                 "\\1"
                 BLENDER_VERSION
                 "${BLENDER_VERSION}")
  find_program(BLENDER_Python3_EXECUTABLE
               NAMES python python.exe
               PATHS "${BLENDER_PATH}/${BLENDER_VERSION}/python/bin"
               NO_DEFAULT_PATH)

  # Right now this is just a shortcut to running Blender, it was supposed to set
  # everything up so exporters were registered and ready to run without making
  # changes to the Blender configuration, but that idea didn't work out in the
  # end. The target is still useful and I may expand on it later on, so it stays
  # for now.
  add_custom_target(
    run-blender
    COMMAND "${BLENDER_EXECUTABLE}"
    DEPENDS generate-python-protobuf-source
    SOURCES ${CMAKE_SOURCE_DIR}/tools/blender/addons/io_mesh_msh/export.py
            ${CMAKE_SOURCE_DIR}/tools/blender/addons/io_skeleton_skl/export.py
            ${CMAKE_SOURCE_DIR}/tools/blender/addons/io_animation_anm/export.py
    COMMENT "Running Blender")
endif()

#
# Clean Build Directory Target
#
if(FIND_EXECUTABLE)
  add_custom_target(
    clean-build
    COMMAND ${FIND_EXECUTABLE} '${CMAKE_BINARY_DIR}' ! -name 'CMakeCache.txt' !
            -wholename '${CMAKE_BINARY_DIR}' -exec rm -rf {} +
    COMMENT "Cleaning Build you will need to rerun cmake configure and generate"
    )
endif()

#
# Generate VS Code Environment
#
if(CMAKE_GENERATOR MATCHES "(MSYS|Unix) Makefiles")
  set(CODE_ZOOMLEVEL "0" CACHE STRING "window.zoomLevel for VS Code.")
  # TODO SOURCE_DIRECTORIES could be set and used to call add_subdirectory
  # directly from this top level
  set(SOURCE_DIRECTORIES ${CMAKE_SOURCE_DIR} application
      # application/windows
      engine
      engine/images
      engine/images/png
      engine/renderers/opengl
      engine/renderers/vulkan
      engine/sound/oggvorbis
      engine/sound/portaudio
      tools/aeontool
      tools/worldeditor)

  set(INCLIST "")

  if(Qt5_FOUND)
    list(APPEND INCLIST ${Qt5Core_INCLUDE_DIRS})
    list(APPEND INCLIST ${Qt5Gui_INCLUDE_DIRS})
    list(APPEND INCLIST ${Qt5Widgets_INCLUDE_DIRS})
  endif(Qt5_FOUND)
  list(APPEND INCLIST ${GLSlang_INCLUDE_DIR})
  list(APPEND INCLIST ${VULKAN_INCLUDE_DIR})
  list(APPEND INCLIST ${Protobuf_INCLUDE_DIR})
  list(APPEND INCLIST ${CMAKE_BINARY_DIR}/engine)
  list(APPEND INCLIST ${CMAKE_BINARY_DIR}/engine/renderers/opengl)

  list(REMOVE_DUPLICATES INCLIST)
  set(INCLIST "\"${INCLIST}\"")
  string(REPLACE ";"
                 "\",\n\""
                 INCLIST
                 "${INCLIST}")

  find_program(GDB_EXECUTABLE gdb HINTS ENV MINGW_PREFIX MSYS2_PATH)
  set(DEBUG_PATH "${CMAKE_BINARY_DIR}/bin")
  if(CMAKE_GENERATOR MATCHES "MSYS Makefiles")
    set(DEBUG_PATH "${DEBUG_PATH};$ENV{MINGW_PREFIX}/bin")
    set(USE_EXTERNAL_CONSOLE "true")
  else()
    set(DEBUG_PATH "$ENV{PATH}:${DEBUG_PATH}")
    set(USE_EXTERNAL_CONSOLE "false")
    set(QT_QPA_PLATFORM "{\"name\":\"QT_QPA_PLATFORM\",\"value\":\"xcb\"},")
  endif()
  configure_file("${CMAKE_SOURCE_DIR}/cmake/tasks.json.in"
                 "${CMAKE_SOURCE_DIR}/.vscode/tasks.json")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/settings.json.in"
                 "${CMAKE_SOURCE_DIR}/.vscode/settings.json")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/launch.json.in"
                 "${CMAKE_SOURCE_DIR}/.vscode/launch.json" @ONLY)
  configure_file("${CMAKE_SOURCE_DIR}/cmake/c_cpp_properties.json.in"
                 "${CMAKE_SOURCE_DIR}/.vscode/c_cpp_properties.json" @ONLY)
endif()

set(COVER_BACKGROUND "${CMAKE_BINARY_DIR}/wallpaper.png"
    CACHE FILEPATH "Background for the create-cover target")
set(COVER_POINTSIZE "250"
    CACHE STRING "Font pointsize for the create-cover target")
add_custom_target(create-cover
                  COMMAND "${ImageMagick_convert_EXECUTABLE}" -size 1920x1080
                          xc:none -background transparent -font Ego.ttf -gravity
                          center -pointsize ${COVER_POINTSIZE} -fill none
                          -stroke SteelBlue4 -strokewidth 10 -draw \"text 0,0
                          \'`date \+\"%B %d %Y\"`\'\" -blur 0x10 -pointsize
                          ${COVER_POINTSIZE} -fill white -stroke none
                          -strokewidth 0 -draw \"text 0,0 \'`date \+\"%B %d
                          %Y\"`\'\" PNG32:overlay.png
                  COMMAND "${ImageMagick_convert_EXECUTABLE}"
                          "${COVER_BACKGROUND}" -draw
                          'image over 0,0 1920,1080 overlay.png' cover.png
                  DEPENDS "${COVER_BACKGROUND}"
                  COMMENT "Creating Cover")

get_property(PLUGIN_LIST GLOBAL PROPERTY PLUGINS)
foreach(PLUGIN ${PLUGIN_LIST})
  string(APPEND PLUGINS "Plugin: \"${PLUGIN}\"\n")
endforeach()
string(REGEX REPLACE "\n$" "" PLUGINS "${PLUGINS}")
configure_file("${CMAKE_SOURCE_DIR}/cmake/config.in" "${CMAKE_SOURCE_DIR}/game/config" @ONLY)

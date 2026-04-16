# Copyright 2016,2019,2021 Rodrigo Jose Hernandez Cordoba
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

set(EXECUTABLE_HEADERS Window.h)
set(EXECUTABLE_SOURCES Main.cpp)

if(UNIX AND NOT APPLE)
  find_package(X11 REQUIRED)
  find_package(OpenGL REQUIRED)
  list(APPEND EXECUTABLE_SOURCES linux.cpp)
elseif(APPLE)
  list(APPEND EXECUTABLE_SOURCES macos.mm)
elseif(WIN32)
  list(APPEND EXECUTABLE_SOURCES windows.cpp)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(game ${EXECUTABLE_HEADERS} ${EXECUTABLE_SOURCES})
if(APPLE)
  target_link_libraries(game "-framework Cocoa" AeonEngine)
else()
  target_link_libraries(game ${X11_LIBRARIES} ${OPENGL_LIBRARIES} AeonEngine)
endif()
add_dependencies(game AeonEngine)

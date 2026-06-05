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
  # macos.mm is Objective-C++; the PCH inherited from AeonGUI is compiled as
  # plain C++ (Objective-C disabled), which triggers "Objective-C was disabled
  # in PCH file but is currently enabled". Skip PCH for this source file.
  set_source_files_properties(macos.mm PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
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

# fps.xhtml is an application specific document, so the application (not the
# shared AeonGUI overlay/plugin) owns it. Compile it into a CompiledDocument
# (FpsDocument) with the xmlcxx tool and load it from Main.cpp via the generic
# GuiOverlay::LoadCompiledDocument seam. xmlcxx only exists when AeonGUI is
# built as a subproject (FetchContent path); otherwise the overlay is empty.
if(USE_AEONGUI AND TARGET xmlcxx)
  if(DEFINED aeongui_SOURCE_DIR)
    list(APPEND CMAKE_MODULE_PATH "${aeongui_SOURCE_DIR}/cmake")
  endif()
  include(xmlcxx)
  xmlcxx_generate(FPS_DOCUMENT_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/fps.xhtml"
    CLASS FpsDocument
    NAMESPACE AeonGames)
  target_sources(game PRIVATE ${FPS_DOCUMENT_SRCS})
  target_include_directories(game PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_compile_definitions(game PRIVATE AEONGUI_HAVE_COMPILED_FPS)
  target_link_libraries(game AeonGUI)
endif()

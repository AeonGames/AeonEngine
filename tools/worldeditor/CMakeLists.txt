# Copyright (C) 2016,2018 Rodrigo Jose Hernandez Cordoba
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    models
                    delegates
                    ${CMAKE_SOURCE_DIR}/include
                    ${PROTOBUF_INCLUDE_DIR}
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS}
                    ${Qt5Widgets_INCLUDE_DIRS})

set(WORLDEDITOR_QT_HEADERS
    models/SceneModel.h
    models/ListModel.h
    models/NodeDataListModel.h
    models/MessageModel.h
    models/PropertyModel.h
    models/NodeDataModel.h
    delegates/PropertyDelegate.h
    RendererSelectDialog.h
    SettingsDialog.h
    MainWindow.h
    SceneWindow.h
    EngineWindow.h)

set(WORLDEDITOR_HEADERS
    Debug.h
    WorldEditor.h
    GridSettings.h
    ${WORLDEDITOR_QT_HEADERS})

set(WORLDEDITOR_SOURCES
    models/SceneModel.cpp
    models/ListModel.cpp
    models/NodeDataListModel.cpp
    models/MessageModel.cpp
    models/PropertyModel.cpp
    models/NodeDataModel.cpp
    delegates/PropertyDelegate.cpp
    Main.cpp
    WorldEditor.cpp
    MainWindow.cpp
    SceneWindow.cpp
    RendererSelectDialog.cpp
    SettingsDialog.cpp
    EngineWindow.cpp
    Debug.cpp
    GridSettings.cpp)

set(WORLDEDITOR_UI
    ui/MainWindow.ui
    ui/SceneWindow.ui
    ui/RendererSelectDialog.ui
    ui/SettingsDialog.ui
    ui/ResourcesDialog.ui)

if(WIN32)
  include_directories(${Qt5WinExtras_INCLUDE_DIRS})
  list(APPEND WORLDEDITOR_SOURCES ${WORLDEDITOR_SOURCES} icon.rc)
endif(WIN32)

configure_file(worldeditor_dynamic.qrc
               ${CMAKE_CURRENT_BINARY_DIR}/worldeditor_dynamic.qrc)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/aabb.msh
         ${CMAKE_CURRENT_BINARY_DIR}/aabb_wire.msh
         ${CMAKE_CURRENT_BINARY_DIR}/grid.msh
  COMMAND $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_CURRENT_SOURCE_DIR}/resources/meshes/aabb.txt -o aabb.msh
  COMMAND $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_CURRENT_SOURCE_DIR}/resources/meshes/aabb_wire.txt -o
          aabb_wire.msh
  COMMAND $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_CURRENT_SOURCE_DIR}/resources/meshes/grid.txt -o grid.msh
  DEPENDS resources/meshes/aabb.txt resources/meshes/aabb_wire.txt
          resources/meshes/grid.txt
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Creating binary meshes.")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/solid_color.prg
         ${CMAKE_CURRENT_BINARY_DIR}/solid_wire.prg
         ${CMAKE_CURRENT_BINARY_DIR}/grid.prg
  COMMAND $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_CURRENT_SOURCE_DIR}/resources/pipelines/solid_color.txt -o
          solid_color.prg
  COMMAND $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_CURRENT_SOURCE_DIR}/resources/pipelines/solid_wire.txt -o
          solid_wire.prg
  COMMAND $<TARGET_FILE:aeontool> convert -i
          ${CMAKE_CURRENT_SOURCE_DIR}/resources/pipelines/grid.txt -o grid.prg
  DEPENDS resources/pipelines/solid_color.txt resources/pipelines/solid_wire.txt
          resources/pipelines/grid.txt
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Creating binary pipelines.")

qt5_wrap_cpp(WORLDEDITOR_MOCS ${WORLDEDITOR_QT_HEADERS})
qt5_wrap_ui(WORLDEDITOR_UI_HEADERS ${WORLDEDITOR_UI})
qt5_add_resources(WORLDEDITOR_RESOURCES worldeditor_static.qrc
                  ${CMAKE_CURRENT_BINARY_DIR}/worldeditor_dynamic.qrc)

add_executable(worldeditor
               ${WORLDEDITOR_SOURCES}
               ${WORLDEDITOR_MOCS}
               ${WORLDEDITOR_UI_HEADERS}
               ${WORLDEDITOR_HEADERS}
               ${WORLDEDITOR_RESOURCES})

if(UNIX OR MINGW OR MSYS)
  set_target_properties(worldeditor PROPERTIES COMPILE_FLAGS "-Wall -Werror")
endif()

if(MSVC)
  set_target_properties(worldeditor
                        PROPERTIES COMPILE_FLAGS "-D_CRT_SECURE_NO_WARNINGS")
endif()

target_link_libraries(worldeditor
                      AeonEngine
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Gui_LIBRARIES}
                      ${Qt5Widgets_LIBRARIES}
                      ${Qt5WinExtras_LIBRARIES}
                      ${OPENGL_LIBRARIES}
                      ${CMAKE_DL_LIBS}
                      ${CMAKE_THREAD_LIBS_INIT})

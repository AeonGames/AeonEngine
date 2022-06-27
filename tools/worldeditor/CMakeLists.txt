# Copyright (C) 2016,2018,2022 Rodrigo Jose Hernandez Cordoba
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
                    ${CMAKE_BINARY_DIR}/engine
                    ${CMAKE_CURRENT_BINARY_DIR}
                    models
                    delegates
                    ${CMAKE_SOURCE_DIR}/include
                    ${PROTOBUF_INCLUDE_DIR}
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS}
                    ${Qt5Widgets_INCLUDE_DIRS})

qt5_create_translation(QM_FILES
                       ${CMAKE_CURRENT_SOURCE_DIR}
                       OPTIONS
                       -source-language
                       en_US
                       -target-language
                       es_CR)

set(WORLDEDITOR_QT_HEADERS
    models/SceneModel.h
    models/ListModel.h
    models/ComponentListModel.h
    models/PropertyModel.h
    models/ComponentModel.h
    delegates/PropertyDelegate.h
    RendererSelectDialog.h
    SettingsDialog.h
    CameraSettings.h
    NodeWindow.h
    NodeView.h
    MainWindow.h
    SceneWindow.h
    EngineWindow.h)

set(WORLDEDITOR_HEADERS
    WorldEditor.h
    ${WORLDEDITOR_QT_HEADERS})

set(WORLDEDITOR_SOURCES
    models/SceneModel.cpp
    models/ListModel.cpp
    models/ComponentListModel.cpp
    models/PropertyModel.cpp
    models/ComponentModel.cpp
    delegates/PropertyDelegate.cpp
    Main.cpp
    WorldEditor.cpp
    MainWindow.cpp
    SceneWindow.cpp
    RendererSelectDialog.cpp
    CameraSettings.cpp
    NodeView.cpp
    NodeWindow.cpp
    SettingsDialog.cpp
    EngineWindow.cpp)

set(WORLDEDITOR_UI
    ui/MainWindow.ui
    ui/SceneWindow.ui
    ui/RendererSelectDialog.ui
    ui/SettingsDialog.ui
    ui/ResourcesDialog.ui
    ui/CameraSettings.ui
    ui/NodeWindow.ui
)

if(WIN32)
  include_directories(${Qt5WinExtras_INCLUDE_DIRS})
  list(APPEND WORLDEDITOR_SOURCES ${WORLDEDITOR_SOURCES} icon.rc)
elseif(UNIX)
  include_directories(${Qt5X11Extras_INCLUDE_DIRS})
endif()

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
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/solid_color.pln
               ${CMAKE_CURRENT_BINARY_DIR}/solid_wire.pln
               ${CMAKE_CURRENT_BINARY_DIR}/grid.pln
        COMMAND $<TARGET_FILE:aeontool> convert -i
                ${CMAKE_CURRENT_SOURCE_DIR}/resources/pipelines/solid_color.txt -o
                solid_color.pln
        COMMAND $<TARGET_FILE:aeontool> convert -i
                ${CMAKE_CURRENT_SOURCE_DIR}/resources/pipelines/solid_wire.txt -o
                solid_wire.pln
        COMMAND $<TARGET_FILE:aeontool> convert -i
                ${CMAKE_CURRENT_SOURCE_DIR}/resources/pipelines/grid.txt -o grid.pln
        DEPENDS resources/pipelines/solid_color.txt resources/pipelines/solid_wire.txt
                resources/pipelines/grid.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Creating binary pipelines.")
add_custom_command(
        OUTPUT
                ${CMAKE_CURRENT_BINARY_DIR}/grid.mtl
                ${CMAKE_CURRENT_BINARY_DIR}/solidcolor.mtl
        COMMAND $<TARGET_FILE:aeontool> convert -i
                ${CMAKE_CURRENT_SOURCE_DIR}/resources/materials/grid.txt -o grid.mtl
        COMMAND $<TARGET_FILE:aeontool> convert -i
                ${CMAKE_CURRENT_SOURCE_DIR}/resources/materials/solidcolor.txt -o solidcolor.mtl
        DEPENDS
                resources/materials/grid.txt
                resources/materials/solidcolor.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Creating binary materials.")

qt5_wrap_cpp(WORLDEDITOR_MOCS ${WORLDEDITOR_QT_HEADERS})
qt5_wrap_ui(WORLDEDITOR_UI_HEADERS ${WORLDEDITOR_UI})
qt5_add_resources(WORLDEDITOR_RESOURCES worldeditor_static.qrc
                  ${CMAKE_CURRENT_BINARY_DIR}/worldeditor_dynamic.qrc)

add_executable(worldeditor
               ${WORLDEDITOR_SOURCES}
               ${WORLDEDITOR_MOCS}
               ${WORLDEDITOR_UI_HEADERS}
               ${WORLDEDITOR_HEADERS}
               ${WORLDEDITOR_RESOURCES}
               ${QM_FILES})

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
                      ${Qt5X11Extras_LIBRARIES}
                      ${OPENGL_LIBRARIES}
                      ${CMAKE_DL_LIBS}
                      ${CMAKE_THREAD_LIBS_INIT})

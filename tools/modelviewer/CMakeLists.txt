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

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}/include
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS}
                    ${Qt5Widgets_INCLUDE_DIRS})

set(MODELVIEWER_QT_HEADERS
    MainWindow.h
    EngineWindow.h
    )

set(MODELVIEWER_HEADERS
    ModelViewer.h
    ${MODELVIEWER_QT_HEADERS})

set(MODELVIEWER_SOURCES
    Main.cpp
    ModelViewer.cpp
    MainWindow.cpp
    EngineWindow.cpp
    )

set(MODELVIEWER_UI
    ui/MainWindow.ui
    )


if(WIN32)
    include_directories(${Qt5WinExtras_INCLUDE_DIRS})
    list(APPEND MODELVIEWER_SOURCES ${MODELVIEWER_SOURCES} icon.rc)
endif(WIN32)

QT5_WRAP_CPP(MODELVIEWER_MOCS ${MODELVIEWER_QT_HEADERS})
QT5_WRAP_UI(MODELVIEWER_UI_HEADERS ${MODELVIEWER_UI})
qt5_add_resources(MODELVIEWER_RESOURCES modelviewer.qrc)

add_executable(modelviewer
    ${MODELVIEWER_SOURCES}
    ${MODELVIEWER_MOCS}
    ${MODELVIEWER_UI_HEADERS}
    ${MODELVIEWER_HEADERS}
    ${MODELVIEWER_RESOURCES})

if(UNIX)
    set_target_properties(modelviewer PROPERTIES COMPILE_FLAGS "-Wall -Werror -O0 -g")
endif()

if(MSVC)
    set_target_properties(modelviewer PROPERTIES COMPILE_FLAGS "-D_CRT_SECURE_NO_WARNINGS")
endif()

target_link_libraries(modelviewer
    AeonEngine
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5WinExtras_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${CMAKE_THREAD_LIBS_INIT})

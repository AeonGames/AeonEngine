# Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba
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

set(AEONGAMESGUI_HEADERS AeonGamesGUI.h)
set(AEONGAMESGUI_SOURCES AeonGamesGUI.cpp Plugin.cpp)
add_library(AeonGamesGUI SHARED ${AEONGAMESGUI_HEADERS} ${AEONGAMESGUI_SOURCES})

if(MSVC)
  set_target_properties(
    AeonGamesGUI
    PROPERTIES COMPILE_FLAGS
               "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DDLL_EXPORT")
elseif(MINGW OR MSYS)
  set_target_properties(AeonGamesGUI
                        PROPERTIES COMPILE_FLAGS
                                   "-Wall -DNOMINMAX -Werror -Wfatal-errors"
                                   PREFIX
                                   "")
endif()
target_link_libraries(AeonGamesGUI AeonEngine AeonGUI)

set_property(GLOBAL APPEND PROPERTY PLUGINS AeonGamesGUI)

if(USE_CLANG_TIDY)
  set_target_properties(
    AeonGamesGUI
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}"
    )
endif()

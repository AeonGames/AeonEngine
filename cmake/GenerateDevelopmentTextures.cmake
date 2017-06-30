# Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba
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

function(generate_svg filename SVG_FILL_COLOR)
    configure_file("${CMAKE_SOURCE_DIR}/game/images/development.svg.in" "${CMAKE_SOURCE_DIR}/game/images/${filename}.svg")
endfunction()
find_program(INKSCAPE_EXECUTABLE NAMES inkscape inkscape.exe HINTS ENV ProgramFiles PATH_SUFFIXES Inkscape)
file(STRINGS ${CMAKE_SOURCE_DIR}/game/colors.txt colors REGEX "[^ \\t]+[ \\t]+#[0-9a-fA-F]+")
foreach(color ${colors})
    string(REGEX REPLACE "([^ \\t]+)[ \\t]+#[0-9a-fA-F]+" "\\1" name ${color})
    string(REGEX REPLACE "[^ \\t]+[ \\t]+(#[0-9a-fA-F]+)" "\\1" hex ${color})
    generate_svg(${name} ${hex})
    if(INKSCAPE_EXECUTABLE)
        execute_process(COMMAND ${INKSCAPE_EXECUTABLE} --export-png=${CMAKE_SOURCE_DIR}/game/images/${name}.png ${CMAKE_SOURCE_DIR}/game/images/${name}.svg)
    endif()    
endforeach()

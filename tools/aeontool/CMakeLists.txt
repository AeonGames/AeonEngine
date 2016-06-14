# Copyright 2016 Rodrigo Jose Hernandez Cordoba
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(AEONTOOL_HEADERS Convert.h)
set(AEONTOOL_SOURCES Main.cpp Convert.cpp)

include_directories(
                    ${PROTOBUF_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_BINARY_DIR}
)

PROTOBUF_GENERATE_CPP(PROTOBUF_SOURCES PROTOBUF_HEADERS
    ${CMAKE_SOURCE_DIR}/proto/Configuration.proto
    ${CMAKE_SOURCE_DIR}/proto/vector2.proto    
    ${CMAKE_SOURCE_DIR}/proto/vector3.proto
    ${CMAKE_SOURCE_DIR}/proto/vector4.proto
    ${CMAKE_SOURCE_DIR}/proto/mesh.proto
    ${CMAKE_SOURCE_DIR}/proto/program.proto
)

source_group("protobuf" FILES ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS})

if(MSVC)
    set_source_files_properties(${PROTOBUF_SOURCES} PROPERTIES COMPILE_FLAGS "-D_SCL_SECURE_NO_WARNINGS -wd4251 -wd4244 -wd4267")
endif()

add_executable(aeontool ${AEONTOOL_HEADERS} ${AEONTOOL_SOURCES} ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS})
target_link_libraries(aeontool ${PROTOBUF_LIBRARIES})
if(MSVC)
    set_target_properties(aeontool PROPERTIES
        COMPILE_FLAGS "-WX -DPROTOBUF_USE_DLLS -D_CRT_SECURE_NO_WARNINGS")
elseif(MINGW)
    set_target_properties(aeontool PROPERTIES
        COMPILE_FLAGS "-Wall -DPROTOBUF_USE_DLLS")
elseif(UNIX)
    set_target_properties(aeontool PROPERTIES
        COMPILE_FLAGS "-Wall")
endif()


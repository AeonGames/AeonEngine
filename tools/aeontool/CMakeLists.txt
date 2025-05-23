# Copyright (C) 2016,2018,2024 Rodrigo Jose Hernandez Cordoba
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

set(AEONTOOL_HEADERS Tool.h Convert.h PipelineTool.h Pack.h Base64.h)
set(AEONTOOL_SOURCES Main.cpp Convert.cpp PipelineTool.cpp Pack.cpp Base64.cpp)

find_package(LibXml2 REQUIRED)

include_directories(${LIBXML2_INCLUDE_DIR}
                    ${ZLIB_INCLUDE_DIR}
                    ${PROTOBUF_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_BINARY_DIR}/proto
                    ${CMAKE_BINARY_DIR}/engine)

add_executable(aeontool ${AEONTOOL_HEADERS} ${AEONTOOL_SOURCES})
target_link_libraries(aeontool ProtoBufClasses AeonEngine ${ZLIB_LIBRARIES} ${LIBXML2_LIBRARIES})
if(MSVC)
  set_target_properties(
    aeontool
    PROPERTIES COMPILE_FLAGS "-WX -D_CRT_SECURE_NO_WARNINGS")
endif()

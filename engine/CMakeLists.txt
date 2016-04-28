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

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${VULKAN_INCLUDE_DIR}
	${PROTOBUF_INCLUDE_DIR}	
)

set(ENGINE_HEADERS 
	${CMAKE_SOURCE_DIR}/include/aeongames/Platform.h
	${CMAKE_SOURCE_DIR}/include/aeongames/AeonEngine.h
	core/Vulkan.h
	core/Configuration.h
)

set(ENGINE_SOURCES 
	core/AeonEngine.cpp
	core/Configuration.cpp
	core/Vulkan.cpp
)

PROTOBUF_GENERATE_CPP(PROTOBUF_SOURCES PROTOBUF_HEADERS
    ${CMAKE_SOURCE_DIR}/proto/Configuration.proto
)

if(MSVC)
    set_source_files_properties(${PROTOBUF_SOURCES} PROPERTIES COMPILE_FLAGS "-D_SCL_SECURE_NO_WARNINGS -wd4251 -wd4244 -wd4267")
endif()

add_library(engine SHARED ${ENGINE_HEADERS} ${ENGINE_SOURCES} ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS})
target_link_libraries(engine ${VULKAN_LIBRARY} ${PROTOBUF_LIBRARIES})

if(MSVC)
	set_target_properties(engine PROPERTIES 
		COMPILE_FLAGS "-WX -DVK_USE_PLATFORM_WIN32_KHR -DDLL_EXPORT -DPROTOBUF_USE_DLLS -D_CRT_SECURE_NO_WARNINGS")
elseif(MINGW)
	set_target_properties(engine PROPERTIES 
		COMPILE_FLAGS "-Wall -DVK_USE_PLATFORM_WIN32_KHR -DDLL_EXPORT -DPROTOBUF_USE_DLLS")
endif()

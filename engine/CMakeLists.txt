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
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/engine/renderers/opengl
    ${CMAKE_SOURCE_DIR}/engine/renderers/vulkan
    ${CMAKE_SOURCE_DIR}/include
    ${VULKAN_INCLUDE_DIR}
    ${PROTOBUF_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

PROTOBUF_GENERATE_EXPORT_CPP(PROTOBUF_SOURCES PROTOBUF_HEADERS "DLL_PROTOBUF"
    ${CMAKE_SOURCE_DIR}/proto/configuration.proto
    ${CMAKE_SOURCE_DIR}/proto/vector2.proto
    ${CMAKE_SOURCE_DIR}/proto/vector3.proto
    ${CMAKE_SOURCE_DIR}/proto/vector4.proto
    ${CMAKE_SOURCE_DIR}/proto/property.proto
    ${CMAKE_SOURCE_DIR}/proto/mesh.proto
    ${CMAKE_SOURCE_DIR}/proto/program.proto
    ${CMAKE_SOURCE_DIR}/proto/material.proto
    ${CMAKE_SOURCE_DIR}/proto/model.proto
)

set(ENGINE_SDK_HEADERS
    ${CMAKE_SOURCE_DIR}/include/aeongames/Platform.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/LogLevel.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Mesh.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Material.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Program.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Model.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Node.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Image.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Texture.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Utilities.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ResourceCache.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ProtoBufClasses.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Memory.h)

set(ENGINE_SCENEGRAPH_HEADERS
    ${CMAKE_SOURCE_DIR}/include/aeongames/Scene.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Node.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Model.h)

set(ENGINE_MATH_HEADERS
    math/Vector4.h
    math/Vector3.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Matrix4x4.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Transform.h)

set(ENGINE_CORE_HEADERS
    include/Factory.h
    include/ProtoBufHelpers.h
    include/Configuration.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/AeonEngine.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h)

set(ENGINE_CORE_SOURCES
    core/Configuration.cpp
    core/Image.cpp
    core/Texture.cpp
    core/Renderer.cpp
    core/Utilities.cpp
    core/AeonEngine.cpp
    core/InitializeFinalize.cpp
    core/LogLevel.cpp
    scenegraph/Model.cpp)

set(ENGINE_SCENEGRAPH_SOURCES
    scenegraph/Scene.cpp
    scenegraph/Node.cpp)

set(ENGINE_MATH_SOURCES
    math/Vector4.cpp
    math/Vector3.cpp
    math/Matrix4x4.cpp
    math/Transform.cpp)

source_group("sdk" FILES ${ENGINE_SDK_HEADERS})
source_group("core" FILES ${ENGINE_CORE_HEADERS} ${ENGINE_CORE_SOURCES})
source_group("scenegraph" FILES ${ENGINE_SCENEGRAPH_HEADERS} ${ENGINE_SCENEGRAPH_SOURCES})
source_group("math" FILES ${ENGINE_MATH_HEADERS} ${ENGINE_MATH_SOURCES})
source_group("protobuf" FILES ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS} ${CMAKE_SOURCE_DIR}/include/aeongames/ProtoBufClasses.h)

set(ENGINE_SOURCES  ${ENGINE_CORE_SOURCES}
                    ${ENGINE_RENDERER_SOURCES}
                    ${ENGINE_SCENEGRAPH_SOURCES}
                    ${ENGINE_MATH_SOURCES})
set(ENGINE_HEADERS  ${ENGINE_SDK_HEADERS}
                    ${ENGINE_CORE_HEADERS}
                    ${ENGINE_RENDERER_HEADERS}
                    ${ENGINE_SCENEGRAPH_HEADERS}
                    ${ENGINE_MATH_HEADERS})

if(MSVC)
    set_source_files_properties(${PROTOBUF_SOURCES} PROPERTIES COMPILE_FLAGS "-D_SCL_SECURE_NO_WARNINGS -wd4251 -wd4244 -wd4267")
endif()

add_library(ProtoBufClasses SHARED ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS})
add_library(AeonEngine SHARED ${ENGINE_HEADERS} ${ENGINE_SOURCES})

target_link_libraries(ProtoBufClasses ${PROTOBUF_LIBRARIES})

if(MSVC)
    set_target_properties(ProtoBufClasses PROPERTIES
        COMPILE_FLAGS "-FIaeongames/ProtoBufClasses.h -WX -DPROTOBUF_USE_DLLS -D_CRT_SECURE_NO_WARNINGS")
    set_target_properties(AeonEngine PROPERTIES
        COMPILE_FLAGS "-WX -D_CRT_SECURE_NO_WARNINGS -DPROTOBUF_USE_DLLS")
    target_link_libraries(AeonEngine ProtoBufClasses)
elseif(MINGW OR MSYS)
    set_target_properties(AeonEngine PROPERTIES
        COMPILE_FLAGS "-Wall  -Werror -Wfatal-errors")
    target_link_libraries(AeonEngine ProtoBufClasses)
elseif(UNIX)
    set_target_properties(AeonEngine PROPERTIES
        COMPILE_FLAGS "-fpic -Wall -Werror -Wfatal-errors")
    target_link_libraries(AeonEngine ProtoBufClasses dl)
endif()

if(CLANG_TIDY_EXECUTABLE)
    add_custom_target(check-engine-code
        COMMAND ${CLANG_TIDY_EXECUTABLE} -fix -header-filter=.* -checks=modernize-* -p ${CMAKE_BINARY_DIR} ${ENGINE_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking Engine Code")
endif()
add_subdirectory(renderers/opengl)
add_subdirectory(renderers/vulkan)
add_subdirectory(images)

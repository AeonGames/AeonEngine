# Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba
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
    ${CMAKE_SOURCE_DIR}/proto/quaternion.proto
    ${CMAKE_SOURCE_DIR}/proto/property.proto
    ${CMAKE_SOURCE_DIR}/proto/mesh.proto
    ${CMAKE_SOURCE_DIR}/proto/skeleton.proto
    ${CMAKE_SOURCE_DIR}/proto/animation.proto
    ${CMAKE_SOURCE_DIR}/proto/pipeline.proto
    ${CMAKE_SOURCE_DIR}/proto/material.proto
    ${CMAKE_SOURCE_DIR}/proto/model.proto
    ${CMAKE_SOURCE_DIR}/proto/modelcontroller.proto
    ${CMAKE_SOURCE_DIR}/proto/geometry.proto
    ${CMAKE_SOURCE_DIR}/proto/transform.proto
    ${CMAKE_SOURCE_DIR}/proto/scene.proto
    ${CMAKE_SOURCE_DIR}/proto/reference.proto
)

set(ENGINE_SDK_HEADERS
    ${CMAKE_SOURCE_DIR}/include/aeongames/Platform.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/LogLevel.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Mesh.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Material.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Pipeline.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Model.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Skeleton.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Animation.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Image.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Sound.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Texture.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Utilities.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ResourceCache.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ProtoBufClasses.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Window.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Memory.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Package.h
)

set(ENGINE_MATH_HEADERS
    ${CMAKE_SOURCE_DIR}/include/aeongames/Vector4.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Vector3.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Vector2.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Quaternion.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Plane.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/AABB.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Frustum.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Matrix4x4.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Transform.h)

set(ENGINE_CORE_HEADERS
    include/Decoder.h
    include/Factory.h
    include/ProtoBufHelpers.h
    include/Configuration.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Component.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Platform.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/DependencyMap.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/FlyWeight.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Scene.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Node.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/AeonEngine.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Mesh.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Pipeline.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Material.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Model.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/SoundSystem.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/CRC.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ProtoBufUtils.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Property.h
)

set(ENGINE_CORE_SOURCES
    core/Configuration.cpp
    core/Image.cpp
    core/Sound.cpp
    core/Texture.cpp
    core/Pipeline.cpp
    core/Material.cpp
    core/Mesh.cpp
    core/Skeleton.cpp
    core/Animation.cpp
    core/Renderer.cpp
    core/SoundSystem.cpp
    core/Utilities.cpp
    core/AeonEngine.cpp
    core/LogLevel.cpp
    core/Model.cpp
    core/Window.cpp
    core/CRC.cpp
    core/Scene.cpp
    core/Node.cpp
    core/Component.cpp
    core/Package.cpp
    core/ProtoBufUtils.cpp
    )

set(ENGINE_MATH_SOURCES
    math/Quaternion.cpp
    math/Vector4.cpp
    math/Vector3.cpp
    math/Vector2.cpp
    math/Plane.cpp
    math/AABB.cpp
    math/Frustum.cpp
    math/Matrix4x4.cpp
    math/Transform.cpp)

source_group("sdk" FILES ${ENGINE_SDK_HEADERS})
source_group("core" FILES ${ENGINE_CORE_HEADERS} ${ENGINE_CORE_SOURCES})
source_group("math" FILES ${ENGINE_MATH_HEADERS} ${ENGINE_MATH_SOURCES})
source_group("protobuf" FILES 
    ${PROTOBUF_SOURCES}
    ${PROTOBUF_HEADERS})

set(ENGINE_SOURCES  ${ENGINE_CORE_SOURCES}
                    ${ENGINE_RENDERER_SOURCES}
                    ${ENGINE_MATH_SOURCES})
set(ENGINE_HEADERS  ${ENGINE_SDK_HEADERS}
                    ${ENGINE_CORE_HEADERS}
                    ${ENGINE_RENDERER_HEADERS}
                    ${ENGINE_MATH_HEADERS})

if(MSVC)
    set_source_files_properties(${PROTOBUF_SOURCES} PROPERTIES COMPILE_FLAGS "-D_SCL_SECURE_NO_WARNINGS -wd4251 -wd4244 -wd4267")
endif()

add_library(ProtoBufClasses SHARED 
    ${PROTOBUF_SOURCES}
    ${PROTOBUF_HEADERS}
    ${CMAKE_SOURCE_DIR}/include/aeongames/ProtoBufClasses.h)
add_library(AeonEngine SHARED ${ENGINE_HEADERS} ${ENGINE_SOURCES})

target_link_libraries(ProtoBufClasses ${PROTOBUF_LIBRARIES})

if(MSVC)
    set_target_properties(ProtoBufClasses PROPERTIES
        COMPILE_FLAGS "-FIaeongames/ProtoBufClasses.h -WX -D_CRT_SECURE_NO_WARNINGS")
    set_target_properties(AeonEngine PROPERTIES
        COMPILE_FLAGS "-WX -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX")
    target_link_libraries(AeonEngine ${ZLIB_LIBRARIES} ProtoBufClasses)
elseif(MINGW OR MSYS)
    set_target_properties(ProtoBufClasses PROPERTIES
        COMPILE_FLAGS "-include aeongames/ProtoBufClasses.h -Wall -Werror -Wfatal-errors")
    set_target_properties(AeonEngine PROPERTIES
        COMPILE_FLAGS "-Wall -Werror -Wfatal-errors -DNOMINMAX")
    target_link_libraries(AeonEngine ${ZLIB_LIBRARIES} ${PROTOBUF_LIBRARIES} ProtoBufClasses stdc++fs)
elseif(UNIX)
    set_target_properties(AeonEngine PROPERTIES
        COMPILE_FLAGS "-fpic -Wall -Werror -Wfatal-errors -DNOMINMAX")
    target_link_libraries(AeonEngine ${ZLIB_LIBRARIES} ProtoBufClasses dl stdc++fs)
endif()

if(USE_CLANG_TIDY)
    set_target_properties(AeonEngine PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}")
endif()

add_subdirectory(components)
add_subdirectory(renderers/opengl)
add_subdirectory(renderers/vulkan)
add_subdirectory(images)
add_subdirectory(sound/portaudio)
add_subdirectory(sound/oggvorbis)

# UI Plugin
if(PKG_CONFIG_FOUND)
    pkg_check_modules(CAIRO cairo)
    if(CAIRO_FOUND)
        message(STATUS "Found Cairo.")
    else()
        message(STATUS "Cairo not found.")
    endif()
endif()

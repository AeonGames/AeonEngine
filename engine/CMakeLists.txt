# Copyright (C) 2016-2021,2023,2024 Rodrigo Jose Hernandez Cordoba
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

include_directories(
                    ${PROTOBUF_INCLUDE_DIR}
                    ${ZLIB_INCLUDE_DIR}
                    ${PNG_PNG_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/engine
                    ${CMAKE_SOURCE_DIR}/engine/include
                    ${CMAKE_SOURCE_DIR}/engine/renderers/opengl
                    ${CMAKE_SOURCE_DIR}/engine/renderers/vulkan
                    ${CMAKE_SOURCE_DIR}/include
                    ${VULKAN_INCLUDE_DIR}
                    ${CMAKE_BINARY_DIR}/proto
)

set(ENGINE_SDK_HEADERS
    ${CMAKE_SOURCE_DIR}/include/aeongames/Platform.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/LogLevel.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/FrameBuffer.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Mesh.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Material.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Pipeline.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Buffer.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/BufferAccessor.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Model.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Skeleton.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Animation.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Texture.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Sound.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Utilities.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ResourceCache.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ResourceFactory.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ResourceId.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ProtoBufClasses.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/MemoryPool.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Package.h)

set(ENGINE_MATH_HEADERS
    ${CMAKE_SOURCE_DIR}/include/aeongames/Vector4.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Vector3.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Vector2.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Quaternion.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Plane.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/AABB.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Frustum.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Matrix4x4.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Matrix3x3.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Transform.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ProtoBufHelpers.h
)

set(ENGINE_CORE_HEADERS
    include/Decoder.h
    include/Factory.h
    include/Configuration.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Platform.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/DependencyMap.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/FlyWeight.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Scene.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Node.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Component.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/StringId.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/AeonEngine.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Pipeline.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Material.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Model.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/SoundSystem.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/CRC.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Base64.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ProtoBufUtils.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Property.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Clock.h
    # ${CMAKE_SOURCE_DIR}/include/aeongames/Octree.h
    )

set(ENGINE_CORE_SOURCES
    core/AeonEngine.cpp
    core/Configuration.cpp
    core/CRC.cpp
    core/Base64.cpp
    core/Component.cpp
    core/Renderer.cpp
    core/Texture.cpp
    core/Sound.cpp
    core/Pipeline.cpp
    core/Material.cpp
    core/Mesh.cpp
    core/Skeleton.cpp
    core/Animation.cpp
    core/SoundSystem.cpp
    core/Utilities.cpp
    core/FrameBuffer.cpp
    core/Model.cpp
    core/Scene.cpp
    core/Node.cpp
    core/Package.cpp
    core/ResourceFactory.cpp
    core/ResourceCache.cpp
    core/Resource.cpp
    core/ProtoBufUtils.cpp
    core/Clock.cpp
    core/MemoryPool.cpp
    core/BufferAccessor.cpp
    # core/Octree.cpp
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
    math/Matrix3x3.cpp
    math/Transform.cpp)

source_group("sdk" FILES ${ENGINE_SDK_HEADERS})
source_group("core" FILES ${ENGINE_CORE_HEADERS} ${ENGINE_CORE_SOURCES})
source_group("math" FILES ${ENGINE_MATH_HEADERS} ${ENGINE_MATH_SOURCES})

set(ENGINE_SOURCES
    ${ENGINE_CORE_SOURCES}
    ${ENGINE_RENDERER_SOURCES}
    ${ENGINE_MATH_SOURCES})
set(ENGINE_HEADERS
    ${ENGINE_SDK_HEADERS}
    ${ENGINE_CORE_HEADERS}
    ${ENGINE_RENDERER_HEADERS}
    ${ENGINE_MATH_HEADERS})

if(MSVC)
  set_source_files_properties(
    ${PROTOBUF_SOURCES} PROPERTIES COMPILE_FLAGS
    # I give up protobuf, you can keep your 4996 warning.
    "-D_SCL_SECURE_NO_WARNINGS -wd4251 -wd4244 -wd4267 -wd4996")
endif()

add_library(AeonEngine SHARED ${ENGINE_HEADERS} ${ENGINE_SOURCES})

if(MSVC)
  set_target_properties(AeonEngine
                        PROPERTIES COMPILE_FLAGS
                                   "-WX -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX")
  target_link_libraries(AeonEngine ${ZLIB_LIBRARIES} ProtoBufClasses)
elseif(MINGW OR MSYS OR UNIX)
  if(NOT ${CMAKE_CXX_COMPILER} MATCHES "clang")
    set(STDXXFS stdc++fs)
  endif()
  target_link_libraries(AeonEngine
                        ${ZLIB_LIBRARIES}
                        ${CMAKE_DL_LIBS}
                        ${STDXXFS}
                        ProtoBufClasses)
endif()

if(USE_CLANG_TIDY)
  set_target_properties(
    AeonEngine
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=${CMAKE_SOURCE_DIR}/include/aeongames;${CLANG_TIDY_CHECKS}"
    )
endif()

if(IWYU_PROGRAM)
  set_property(TARGET AeonEngine PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${IWYU_COMMAND_LINE})
endif()

add_subdirectory(components)
if(BUILD_OPENGL_RENDERER)
  add_subdirectory(renderers/opengl)
endif()
if(BUILD_VULKAN_RENDERER)
  add_subdirectory(renderers/vulkan)
endif()
add_subdirectory(images)
add_subdirectory(sound/portaudio)
add_subdirectory(sound/oggvorbis)

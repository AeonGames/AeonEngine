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
    ${CMAKE_SOURCE_DIR}/engine/renderers/opengl
    ${CMAKE_SOURCE_DIR}/engine/renderers/vulkan
    core/${GUI_PLATFORM}
    ${CMAKE_SOURCE_DIR}/include
    ${VULKAN_INCLUDE_DIR}
    ${PROTOBUF_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command( OUTPUT
                        ${CMAKE_CURRENT_BINARY_DIR}/main.vert.h
                        ${CMAKE_CURRENT_BINARY_DIR}/main.frag.h
                    COMMAND ${XXD_EXECUTABLE} -i main.vert > ${CMAKE_CURRENT_BINARY_DIR}/main.vert.h
                    COMMAND ${XXD_EXECUTABLE} -i main.frag > ${CMAKE_CURRENT_BINARY_DIR}/main.frag.h
                    DEPENDS
                        renderers/opengl/shaders/main.vert
                        renderers/opengl/shaders/main.frag
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/renderers/opengl/shaders
                    COMMENT "Generating Shader Headers" VERBATIM)

set(ENGINE_SDK_HEADERS
    ${CMAKE_SOURCE_DIR}/include/aeongames/Platform.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/LogLevel.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Mesh.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ResourceCache.h)
    
set(ENGINE_VULKAN_RENDERER_HEADERS
    ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h
    renderers/vulkan/VulkanRenderer.h)
    
set(ENGINE_OPENGL_RENDERER_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/main.vert.h
    ${CMAKE_CURRENT_BINARY_DIR}/main.frag.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h
    renderers/opengl/OpenGLRenderer.h
    renderers/opengl/OpenGLFunctions.h
    renderers/opengl/OpenGLPipeline.h    
    renderers/opengl/OpenGLMesh.h)
    
set(ENGINE_CORE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/aeongames/AeonEngine.h
    core/Configuration.h
    core/${GUI_PLATFORM}/GameWindow.h)
    
set(ENGINE_SCENEGRAPH_HEADERS
    scenegraph/Scene.h
    scenegraph/Node.h)
    
set(ENGINE_MATH_HEADERS
    math/Vector4.h
    math/Vector3.h
    math/Matrix4x4.h
    math/Transform.h)

set(ENGINE_CORE_SOURCES
    core/AeonEngine.cpp
    core/LogLevel.cpp
    core/Configuration.cpp
    core/${GUI_PLATFORM}/GameWindow.cpp)
    
set(ENGINE_VULKAN_RENDERER_SOURCES
    renderers/vulkan/VulkanRenderer.cpp)
    
set(ENGINE_OPENGL_RENDERER_SOURCES
    renderers/opengl/${GUI_PLATFORM}/OpenGLRenderer.cpp
    renderers/opengl/OpenGLRenderer.cpp
    renderers/opengl/OpenGLPipeline.cpp
    renderers/opengl/OpenGLMesh.cpp
    renderers/opengl/OpenGLFunctions.cpp)
    
set(ENGINE_SCENEGRAPH_SOURCES
    scenegraph/Scene.cpp
    scenegraph/Node.cpp)
    
set(ENGINE_MATH_SOURCES
    math/Vector4.cpp
    math/Vector3.cpp
    math/Matrix4x4.cpp
    math/Transform.cpp)

PROTOBUF_GENERATE_CPP(PROTOBUF_SOURCES PROTOBUF_HEADERS
    ${CMAKE_SOURCE_DIR}/proto/Configuration.proto
    ${CMAKE_SOURCE_DIR}/proto/mesh.proto
)

source_group("sdk" FILES ${ENGINE_SDK_HEADERS})
source_group("core" FILES ${ENGINE_CORE_HEADERS} ${ENGINE_CORE_SOURCES})
source_group("vulkan" FILES ${ENGINE_VULKAN_RENDERER_HEADERS} ${ENGINE_VULKAN_RENDERER_SOURCES})
source_group("opengl" FILES ${ENGINE_OPENGL_RENDERER_HEADERS} ${ENGINE_OPENGL_RENDERER_SOURCES})
source_group("scenegraph" FILES ${ENGINE_SCENEGRAPH_HEADERS} ${ENGINE_SCENEGRAPH_SOURCES})
source_group("math" FILES ${ENGINE_MATH_HEADERS} ${ENGINE_MATH_SOURCES})
source_group("protobuf" FILES ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS})

set(ENGINE_SOURCES  ${ENGINE_SDK_HEADERS} ${ENGINE_CORE_HEADERS} ${ENGINE_CORE_SOURCES}
                    ${ENGINE_RENDERER_HEADERS} ${ENGINE_RENDERER_SOURCES}
                    ${ENGINE_VULKAN_RENDERER_HEADERS} ${ENGINE_VULKAN_RENDERER_SOURCES}
                    ${ENGINE_OPENGL_RENDERER_HEADERS} ${ENGINE_OPENGL_RENDERER_SOURCES}
                    ${ENGINE_SCENEGRAPH_HEADERS} ${ENGINE_SCENEGRAPH_SOURCES}
                    ${ENGINE_MATH_HEADERS} ${ENGINE_MATH_SOURCES}
                    ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS})

if(MSVC)
    set_source_files_properties(${PROTOBUF_SOURCES} PROPERTIES COMPILE_FLAGS "-D_SCL_SECURE_NO_WARNINGS -wd4251 -wd4244 -wd4267")
endif()

add_library(engine SHARED ${ENGINE_HEADERS} ${ENGINE_SOURCES}
    ${PROTOBUF_SOURCES} ${PROTOBUF_HEADERS}
    )
target_link_libraries(engine ${VULKAN_LIBRARY} ${OPENGL_LIBRARIES} ${PROTOBUF_LIBRARIES})

if(MSVC)
    set_target_properties(engine PROPERTIES
        COMPILE_FLAGS "-WX -DVK_USE_PLATFORM_WIN32_KHR -DDLL_EXPORT -DPROTOBUF_USE_DLLS -D_CRT_SECURE_NO_WARNINGS")
elseif(MINGW)
    set_target_properties(engine PROPERTIES
        COMPILE_FLAGS "-Wall -DVK_USE_PLATFORM_WIN32_KHR -DDLL_EXPORT -DPROTOBUF_USE_DLLS")
elseif(UNIX)
    set_target_properties(engine PROPERTIES
        COMPILE_FLAGS "-Wall -DVK_USE_PLATFORM_XLIB_KHR")
endif()

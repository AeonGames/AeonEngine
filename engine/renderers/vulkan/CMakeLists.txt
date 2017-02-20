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

add_subdirectory(${VULKAN_GLSLANG_SOURCE_DIR} ${VULKAN_GLSLANG_SOURCE_DIR})

set(VULKAN_RENDERER_HEADERS
    VulkanRenderer.h)
set(VULKAN_RENDERER_SOURCES
    VulkanRenderer.cpp
    Plugin.cpp
   ${VULKAN_GLSLANG_SOURCE_DIR}/StandAlone/StandAlone.cpp)

set_source_files_properties(
    ${VULKAN_GLSLANG_SOURCE_DIR}/StandAlone/StandAlone.cpp
        PROPERTIES 
            COMPILE_FLAGS "-Dmain=compile"
)
    
source_group("vulkan" FILES ${VULKAN_RENDERER_HEADERS} ${VULKAN_RENDERER_SOURCES})

add_library(VulkanRenderer SHARED
    ${VULKAN_RENDERER_HEADERS} ${VULKAN_RENDERER_SOURCES}
    ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h)

target_link_libraries(  VulkanRenderer
                        AeonEngine
                        ProtoBufClasses
                        ${VULKAN_LIBRARY}
                        glslang
                        OGLCompiler
                        OSDependent
                        SPIRV
                        SPVRemapper
                        glslang-default-resource-limits
                        HLSL
)

if(MSVC)
    set_target_properties(VulkanRenderer PROPERTIES
        COMPILE_FLAGS "-WX -D_CRT_SECURE_NO_WARNINGS -DVK_USE_PLATFORM_WIN32_KHR")
elseif(MINGW OR MSYS)
    set_target_properties(VulkanRenderer PROPERTIES
        COMPILE_FLAGS "-Wall  -Werror -Wfatal-errors -DVK_USE_PLATFORM_WIN32_KHR")
elseif(UNIX)
    set_target_properties(VulkanRenderer PROPERTIES
        COMPILE_FLAGS "-fpic -Wall  -Werror -Wfatal-errors -DVK_USE_PLATFORM_XLIB_KHR")
endif()

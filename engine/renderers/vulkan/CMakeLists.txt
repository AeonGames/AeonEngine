# Copyright (C) 2016-2017,2020,2021 Rodrigo Jose Hernandez Cordoba
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

find_package(Vulkan)
find_package(GLslang)

if(GLSLANG_HAS_REVISION_H)
    add_definitions(-DGLSLANG_HAS_REVISION_H)
endif()

include_directories(${GLSLANG_SPIRV_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR})

set(SPIRV_HEADERS
    SPIR-V/CompilerLinker.h
    SPIR-V/Worklist.h
)
set(SPIRV_SOURCES SPIR-V/CompilerLinker.cpp)

set(VULKAN_RENDERER_HEADERS
    VulkanUtilities.h
    VulkanRenderer.h
    VulkanWindow.h
    VulkanBuffer.h
    VulkanMesh.h
    VulkanMemoryPoolBuffer.h
    VulkanFrameBuffer.h
    )

set(VULKAN_RENDERER_SOURCES
    VulkanUtilities.cpp
    VulkanRenderer.cpp
    VulkanWindow.cpp
    VulkanBuffer.cpp
    VulkanMesh.cpp
    VulkanMemoryPoolBuffer.cpp
    VulkanFrameBuffer.cpp
    Plugin.cpp)

source_group("Renderer"
             FILES
             ${VULKAN_RENDERER_HEADERS}
             ${VULKAN_RENDERER_SOURCES})
source_group("Compiler" FILES ${SPIRV_HEADERS} ${SPIRV_SOURCES})

add_library(VulkanRenderer
            SHARED
            ${VULKAN_RENDERER_HEADERS}
            ${VULKAN_RENDERER_SOURCES}
            ${SPIRV_HEADERS}
            ${SPIRV_SOURCES}
            ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.h)

target_link_libraries(VulkanRenderer
                      AeonEngine
                      ProtoBufClasses
                      Threads::Threads
                      ${GLSLANG_LIBRARIES}
                      ${VULKAN_LIBRARY}
                      )

if(MSVC)
  set_target_properties(
    VulkanRenderer
    PROPERTIES
      COMPILE_FLAGS
      "-WX -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DVK_USE_PLATFORM_WIN32_KHR")
elseif(MINGW OR MSYS)
  set_target_properties(
    VulkanRenderer
    PROPERTIES
      COMPILE_FLAGS
        "-DVK_USE_PLATFORM_WIN32_KHR"
      PREFIX
        "")
elseif(UNIX)
  set_target_properties(
    VulkanRenderer
    PROPERTIES
      COMPILE_FLAGS
        "-DVK_USE_PLATFORM_XLIB_KHR"
  )
endif()

set_property(GLOBAL APPEND PROPERTY PLUGINS VulkanRenderer)

if(USE_CLANG_TIDY)
  set_target_properties(
    VulkanRenderer
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}"
    )
endif()

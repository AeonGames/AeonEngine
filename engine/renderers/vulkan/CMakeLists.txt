# Copyright (C) 2016-2017,2020,2021,2025 Rodrigo Jose Hernandez Cordoba
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

include(gitclone)

find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found! Please install Vulkan SDK and set VULKAN_SDK environment variable.")
endif()

find_package(glslang REQUIRED)
if(NOT glslang_FOUND)
    message(FATAL_ERROR "glslang not found")
endif()

# Find MoltenVK on macOS
if(APPLE)
    find_library(MOLTENVK_LIBRARY MoltenVK PATHS /opt/homebrew/lib /usr/local/lib)
    if(NOT MOLTENVK_LIBRARY)
        message(FATAL_ERROR "MoltenVK library not found")
    endif()
endif()

gitclone(https://github.com/KhronosGroup/SPIRV-Reflect.git PATH "${CMAKE_CURRENT_BINARY_DIR}/spirv-reflect" TAG vulkan-sdk-1.4.321.0)

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
    VulkanPipeline.h
    VulkanMaterial.h
    VulkanTexture.h
    VulkanMemoryPoolBuffer.h
    VulkanFrameBuffer.h
    )

set(VULKAN_RENDERER_SOURCES
    VulkanUtilities.cpp
    VulkanRenderer.cpp
    VulkanWindow.cpp
    VulkanBuffer.cpp
    VulkanMesh.cpp
    VulkanPipeline.cpp
    VulkanMaterial.cpp
    VulkanTexture.cpp
    VulkanMemoryPoolBuffer.cpp
    VulkanFrameBuffer.cpp
    Plugin.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/spirv-reflect)
set(SPIRV_REFLECT_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/spirv-reflect/spirv_reflect.cpp
)

set(SPIRV_REFLECT_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/spirv-reflect/spirv_reflect.h
)

source_group("Renderer"
            FILES
            ${VULKAN_RENDERER_HEADERS}
            ${VULKAN_RENDERER_SOURCES}
            ${SPIRV_REFLECT_SOURCES}
            ${SPIRV_REFLECT_HEADERS}
)
source_group("Compiler" FILES ${SPIRV_HEADERS} ${SPIRV_SOURCES})

add_library(VulkanRenderer
            SHARED
            ${VULKAN_RENDERER_HEADERS}
            ${VULKAN_RENDERER_SOURCES}
            ${SPIRV_REFLECT_SOURCES}
            ${SPIRV_REFLECT_HEADERS}
            ${SPIRV_HEADERS}
            ${SPIRV_SOURCES}
            ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.hpp
)

if(IWYU_PROGRAM)
  set_property(TARGET VulkanRenderer PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${IWYU_COMMAND_LINE})
endif()

target_link_libraries(VulkanRenderer PRIVATE
                      AeonEngine
                      ProtoBufClasses
                      Threads::Threads
                      Vulkan::Vulkan
                      glslang::glslang
                      glslang::glslang-default-resource-limits
                      glslang::SPIRV
)

# Add platform-specific libraries
if(APPLE)
  target_link_libraries(VulkanRenderer PRIVATE
                        "-framework Metal"
                        "-framework MetalKit"
                        "-framework Foundation"
                        "-framework QuartzCore"
                        "-framework CoreGraphics"
                        "-framework IOKit"
                        "-framework IOSurface"
                        "-framework AppKit"
                        "-framework Cocoa"
                        "-framework CoreFoundation"
                        "-framework CoreVideo"
                        ${MOLTENVK_LIBRARY})
elseif(UNIX AND NOT APPLE)
  target_link_libraries(VulkanRenderer PRIVATE
                        X11)
endif()

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
elseif(APPLE)
  set_target_properties(
    VulkanRenderer
    PROPERTIES
      COMPILE_FLAGS
        "-DVK_USE_PLATFORM_METAL_EXT"
  )
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

# Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba
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

if(NOT APPLE)
  message(FATAL_ERROR "The Metal renderer can only be built on macOS")
endif()

set(METAL_RENDERER_HEADERS
  MetalBindlessResources.h
  MetalBuffer.h
  MetalMaterial.h
  MetalMemoryPoolBuffer.h
  MetalMesh.h
  MetalPipeline.h
  MetalRenderer.h
  MetalTexture.h
  MetalWindow.h)
set(METAL_RENDERER_SOURCES
  MetalBindlessResources.mm
  MetalBuffer.mm
  MetalMaterial.cpp
  MetalMemoryPoolBuffer.cpp
  MetalMesh.mm
  MetalPipeline.mm
  MetalRenderer.mm
  MetalTexture.mm
  MetalWindow.mm
  Plugin.cpp)

source_group("Renderer" FILES ${METAL_RENDERER_HEADERS} ${METAL_RENDERER_SOURCES})

add_library(MetalRenderer SHARED
            ${METAL_RENDERER_HEADERS}
            ${METAL_RENDERER_SOURCES}
            ${CMAKE_SOURCE_DIR}/include/aeongames/Renderer.hpp)

set_source_files_properties(MetalBindlessResources.mm MetalBuffer.mm MetalMesh.mm MetalPipeline.mm MetalRenderer.mm MetalTexture.mm MetalWindow.mm
                            PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_link_libraries(MetalRenderer PRIVATE
                      AeonEngine
                      ProtoBufClasses
                      Threads::Threads
                      "-framework AppKit"
                      "-framework Foundation"
                      "-framework Metal"
                      "-framework QuartzCore")

target_compile_options(MetalRenderer PRIVATE -fobjc-arc)

set_property(GLOBAL APPEND PROPERTY PLUGINS MetalRenderer)

if(USE_CLANG_TIDY)
  set_target_properties(
    MetalRenderer
    PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXECUTABLE};-fix;-header-filter=aeongames/;${CLANG_TIDY_CHECKS}")
endif()
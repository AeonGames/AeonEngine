# Copyright (c) 2002-2006 Marcus Geelnard
# Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
# Copyright (c) 2016,2017 Rodrigo Jose Hernandez Cordoba
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would
#    be appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such, and must not
#    be misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source
#    distribution.

# This file was taken from the glfw project, the copyright notice above
# only applies to this file.
# This is a modified version of the original file
#
# CHANGELOG
# 4/22/2016 Added copyright notice and modification notice.
# 4/28/2016 Added Void Pointer size check to detect 64 bit architecture.
#

# Find Vulkan
#
# VULKAN_INCLUDE_DIR
# VULKAN_SDK_DIR
# VULKAN_FOUND

if (WIN32)
    find_path(VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h HINTS
        "$ENV{VULKAN_SDK}/Include"
        "$ENV{VK_SDK_PATH}/Include")
    if ((CMAKE_CL_64) OR (CMAKE_SIZEOF_VOID_P EQUAL 8))
        find_library(VULKAN_LIBRARY NAMES vulkan-1 HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")
        find_library(VULKAN_GLSLANG_LIBRARY NAMES glslang HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")
        find_library(VULKAN_OGLCOMPILER_LIBRARY NAMES OGLCompiler HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")
        find_library(VULKAN_OSDEPENDENT_LIBRARY NAMES OSDependent HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")
        find_library(VULKAN_HLSL_LIBRARY NAMES HLSL HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")
        find_library(VULKAN_SPIRV_LIBRARY NAMES SPIRV HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")
        find_library(VULKAN_GLSLANG_DRL_LIBRARY NAMES glslang-default-resource-limits HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")

        set(VULKAN_GLSLANG_LIBRARIES
        ${VULKAN_GLSLANG_LIBRARY}
        ${VULKAN_OGLCOMPILER_LIBRARY}
        ${VULKAN_OSDEPENDENT_LIBRARY}
        ${VULKAN_HLSL_LIBRARY}
        ${VULKAN_SPIRV_LIBRARY}
        ${VULKAN_GLSLANG_DRL_LIBRARY}
        CACHE STRING "glslang validator libraries")
            
    else()
        find_library(VULKAN_LIBRARY NAMES vulkan-1 HINTS
            "$ENV{VULKAN_SDK}/Bin32"
            "$ENV{VK_SDK_PATH}/Bin32")
    endif()
else()
    find_path(VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h HINTS
        "$ENV{VULKAN_SDK}/include")
    find_library(VULKAN_LIBRARY NAMES vulkan HINTS
        "$ENV{VULKAN_SDK}/lib")
endif()
if(DEFINED ENV{VK_SDK_PATH})
    set(VULKAN_SDK_DIR "$ENV{VK_SDK_PATH}" CACHE PATH "Vulkan SDK root directory")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan DEFAULT_MSG VULKAN_LIBRARY VULKAN_INCLUDE_DIR)

mark_as_advanced(   VULKAN_INCLUDE_DIR
                    VULKAN_LIBRARY
                    VULKAN_SDK_DIR
                    VULKAN_GLSLANG_LIBRARIES
                    VULKAN_GLSLANG_LIBRARY
                    VULKAN_OGLCOMPILER_LIBRARY
                    VULKAN_OSDEPENDENT_LIBRARY
                    VULKAN_HLSL_LIBRARY
                    VULKAN_SPIRV_LIBRARY
                    VULKAN_GLSLANG_DRL_LIBRARY                    
)

# Copyright 2017 Rodrigo Jose Hernandez Cordoba
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

find_program(GLSLANG_VALIDATOR_EXECUTABLE NAMES glslangValidator)

find_path(GLSLANG_SPIRV_INCLUDE_DIR SPIRV/spirv.hpp)

find_library(GLSLANG_LIB NAMES glslang glslangd)

find_library(OGLCompiler_LIB NAMES OGLCompiler OGLCompilerd)

find_library(OSDependent_LIB NAMES OSDependent OSDependentd)

find_library(HLSL_LIB NAMES HLSL HLSLd)

find_library(SPIRV_LIB NAMES SPIRV SPIRVd)

find_library(SPIRV_REMAPPER_LIB NAMES SPVRemapper SPVRemapperd)

# - Locate Debug Libraries if they exist -

find_library(GLSLANG_DEBUG_LIB NAMES glslangd glslang)

find_library(OGLCompiler_DEBUG_LIB NAMES OGLCompilerd OGLCompiler)

find_library(OSDependent_DEBUG_LIB NAMES OSDependentd OSDependent)

find_library(HLSL_DEBUG_LIB NAMES HLSLd HLSL)

find_library(SPIRV_DEBUG_LIB NAMES SPIRV SPIRVd)

find_library(SPIRV_DEBUG_REMAPPER_LIB NAMES SPVRemapperd SPVRemapper)

set(GLSLANG_LIBRARIES 
    optimized ${GLSLANG_LIB}
    debug ${GLSLANG_DEBUG_LIB}
    optimized ${OGLCompiler_LIB} 
    debug ${OGLCompiler_DEBUG_LIB} 
    optimized ${OSDependent_LIB}
    debug ${OSDependent_DEBUG_LIB}
    optimized ${HLSL_LIB}
    debug ${HLSL_DEBUG_LIB}
    optimized ${SPIRV_LIB}
    debug ${SPIRV_DEBUG_LIB}
    optimized ${SPIRV_REMAPPER_LIB}
    debug ${SPIRV_DEBUG_REMAPPER_LIB}
    CACHE STRING "GLSLANG_LIBRARIES")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLslang 
    DEFAULT_MSG
    GLSLANG_LIB
    OGLCompiler_LIB
    OSDependent_LIB
    HLSL_LIB
    SPIRV_LIB
    SPIRV_REMAPPER_LIB    
    GLSLANG_VALIDATOR_EXECUTABLE
    GLSLANG_SPIRV_INCLUDE_DIR
    GLSLANG_LIBRARIES
)

mark_as_advanced(
    GLSLANG_LIB
    OGLCompiler_LIB
    OSDependent_LIB
    HLSL_LIB
    SPIRV_LIB
    SPIRV_REMAPPER_LIB
)

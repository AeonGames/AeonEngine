# Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba
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

find_program(GLSLANG_VALIDATOR_EXECUTABLE
             NAMES glslangValidator
             PATHS /usr/local
)
           
find_path(GLSLANG_SPIRV_INCLUDE_DIR SPIRV/spirv.hpp
  PATHS /usr/local/include /mingw64/include/ /mingw32/include
  PATH_SUFFIXES glslang
)

find_path(GLSLANG_INCLUDE_DIR ResourceLimits.h
  PATHS /usr/local/include /mingw64/include/ /mingw32/include
  PATH_SUFFIXES glslang/Include
)
      
if(GLSLANG_INCLUDE_DIR AND EXISTS "${GLSLANG_INCLUDE_DIR}/revision.h")
  set(GLSLANG_HAS_REVISION_H ON CACHE BOOL "")
else()
  set(GLSLANG_HAS_REVISION_H OFF CACHE BOOL "")
endif()

find_library(GLSLANG_LIB NAMES glslang)
find_library(OGLCompiler_LIB NAMES OGLCompiler)
find_library(OSDependent_LIB NAMES OSDependent)
find_library(HLSL_LIB NAMES HLSL)
find_library(SPIRV_LIB NAMES SPIRV)
find_library(SPIRV_TOOLS_OPT_LIB NAMES SPIRV-Tools-opt)
find_library(SPIRV_REMAPPER_LIB NAMES SPVRemapper)

# * Locate Debug Libraries if they exist -

find_library(GLSLANG_DEBUG_LIB NAMES glslangd)
find_library(OGLCompiler_DEBUG_LIB NAMES OGLCompilerd)
find_library(OSDependent_DEBUG_LIB NAMES OSDependentd)
find_library(HLSL_DEBUG_LIB NAMES HLSLd)
find_library(SPIRV_DEBUG_LIB NAMES SPIRVd)
find_library(SPIRV_TOOLS_OPT_DEBUG_LIB NAMES SPIRV-Tools-optd)
find_library(SPIRV_REMAPPER_DEBUG_LIB NAMES SPVRemapperd)

if(NOT GLSLANG_HAS_REVISION_H)
  find_library(MACHINEINDEPENDENT_LIB MachineIndependent)
  find_library(MACHINEINDEPENDENT_DEBUG_LIB MachineIndependentd)
  find_library(GENERICCODEGEN_DEBUG_LIB GenericCodeGend)
  find_library(GENERICCODEGEN_LIB GenericCodeGen)

  if(MACHINEINDEPENDENT_LIB AND MACHINEINDEPENDENT_DEBUG_LIB)
    list(APPEND GLSLANG_LIBRARIES
                optimized
                ${MACHINEINDEPENDENT_LIB}
                debug
                ${MACHINEINDEPENDENT_DEBUG_LIB})
  elseif(MACHINEINDEPENDENT_LIB)
    list(APPEND GLSLANG_LIBRARIES ${MACHINEINDEPENDENT_LIB})
  elseif(SPIRV_REMAPPER_DEBUG_LIB)
    list(APPEND GLSLANG_LIBRARIES ${MACHINEINDEPENDENT_DEBUG_LIB})
  endif()

  if(GENERICCODEGEN_LIB AND GENERICCODEGEN_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES
              optimized
              ${GENERICCODEGEN_LIB}
              debug
              ${GENERICCODEGEN_DEBUG_LIB})
  elseif(GENERICCODEGEN_LIB)
    list(APPEND GLSLANG_LIBRARIES ${GENERICCODEGEN_LIB})
  elseif(SPIRV_REMAPPER_DEBUG_LIB)
    list(APPEND GLSLANG_LIBRARIES ${GENERICCODEGEN_DEBUG_LIB})
  endif()

endif()

if(GLSLANG_LIB AND GLSLANG_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES
              optimized
              ${GLSLANG_LIB}
              debug
              ${GLSLANG_DEBUG_LIB})
elseif(GLSLANG_LIB)
  list(APPEND GLSLANG_LIBRARIES ${GLSLANG_LIB})
elseif(GLSLANG_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES ${GLSLANG_DEBUG_LIB})
endif()

if(OGLCompiler_LIB AND OGLCompiler_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES
              optimized
              ${OGLCompiler_LIB}
              debug
              ${OGLCompiler_DEBUG_LIB})
elseif(OGLCompiler_LIB)
  list(APPEND GLSLANG_LIBRARIES ${OGLCompiler_LIB})
elseif(OGLCompiler_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES ${OGLCompiler_DEBUG_LIB})
endif()

if(OSDependent_LIB AND OSDependent_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES
              optimized
              ${OSDependent_LIB}
              debug
              ${OSDependent_DEBUG_LIB})
elseif(OSDependent_LIB)
  list(APPEND GLSLANG_LIBRARIES ${OSDependent_LIB})
elseif(OSDependent_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES ${OSDependent_DEBUG_LIB})
endif()

if(HLSL_LIB AND HLSL_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES
              optimized
              ${HLSL_LIB}
              debug
              ${HLSL_DEBUG_LIB})
elseif(HLSL_LIB)
  list(APPEND GLSLANG_LIBRARIES ${HLSL_LIB})
elseif(HLSL_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES ${HLSL_DEBUG_LIB})
endif()

if(SPIRV_LIB AND SPIRV_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES
              optimized
              ${SPIRV_LIB}
              debug
              ${SPIRV_DEBUG_LIB})
elseif(SPIRV_LIB)
  list(APPEND GLSLANG_LIBRARIES ${SPIRV_LIB})
elseif(SPIRV_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES ${SPIRV_DEBUG_LIB})
endif()

if(SPIRV_TOOLS_OPT_LIB AND SPIRV_TOOLS_OPT_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES
              optimized
              ${SPIRV_TOOLS_OPT_LIB}
              debug
              ${SPIRV_TOOLS_OPT_DEBUG_LIB})
elseif(SPIRV_TOOLS_OPT_LIB)
  list(APPEND GLSLANG_LIBRARIES ${SPIRV_TOOLS_OPT_LIB})
elseif(SPIRV_TOOLS_OPT_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES ${SPIRV_TOOLS_OPT_DEBUG_LIB})
endif()

if(SPIRV_REMAPPER_LIB AND SPIRV_REMAPPER_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES
              optimized
              ${SPIRV_REMAPPER_LIB}
              debug
              ${SPIRV_REMAPPER_DEBUG_LIB})
elseif(SPIRV_REMAPPER_LIB)
  list(APPEND GLSLANG_LIBRARIES ${SPIRV_REMAPPER_LIB})
elseif(SPIRV_REMAPPER_DEBUG_LIB)
  list(APPEND GLSLANG_LIBRARIES ${SPIRV_REMAPPER_DEBUG_LIB})
endif()

include(FindPackageHandleStandardArgs)

set(VARIABLES
  GLSLANG_LIB
  OGLCompiler_LIB
  OSDependent_LIB
  HLSL_LIB
  SPIRV_LIB
  SPIRV_REMAPPER_LIB
  GLSLANG_VALIDATOR_EXECUTABLE
  GLSLANG_INCLUDE_DIR
  GLSLANG_SPIRV_INCLUDE_DIR
  GLSLANG_LIBRARIES
)

if(NOT GLSLANG_HAS_REVISION_H)
  list(APPEND VARIABLES MACHINEINDEPENDENT_LIB)
endif()

find_package_handle_standard_args(GLslang REQUIRED_VARS
  ${VARIABLES}
)

mark_as_advanced(
  GLSLANG_HAS_REVISION_H
  ${VARIABLES}
)

# Copyright (C) 2017,2018,2021 Rodrigo Jose Hernandez Cordoba
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

set(GLSLANG_LIBRARIES)
set(GLSLANG_VARIABLES)

function(find_glslang_library library_name)
  string(TOUPPER ${library_name} library_variable)
  string(REPLACE "-" "_" library_variable "${library_variable}")
  list(APPEND GLSLANG_VARIABLES "${library_variable}_LIB")
  set(GLSLANG_VARIABLES "${GLSLANG_VARIABLES}" PARENT_SCOPE)

  find_library(${library_variable}_DEBUG_LIB ${library_name}d)
  find_library(${library_variable}_LIB ${library_name})

  if(${library_variable}_LIB AND ${library_variable}_DEBUG_LIB)
    list(APPEND GLSLANG_LIBRARIES
                optimized
                ${${library_variable}_LIB}
                debug
                ${${library_variable}_DEBUG_LIB})
  elseif(${library_variable}_LIB)
    list(APPEND GLSLANG_LIBRARIES ${${library_variable}_LIB})
  elseif(${library_variable}_DEBUG_LIB)
    list(APPEND GLSLANG_LIBRARIES ${${library_variable}_DEBUG_LIB})
  endif()
  set(GLSLANG_LIBRARIES "${GLSLANG_LIBRARIES}" PARENT_SCOPE)
endfunction()

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

find_glslang_library(HLSL)
find_glslang_library(SPIRV)
find_glslang_library(glslang)
find_glslang_library(OGLCompiler)
find_glslang_library(OSDependent)
find_glslang_library(SPIRV-Tools-opt)
find_glslang_library(SPVRemapper)
find_glslang_library(glslang-default-resource-limits)
if(MSVC)
  find_glslang_library(MachineIndependent)
  find_glslang_library(GenericCodeGen)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GLslang REQUIRED_VARS
  ${GLSLANG_VARIABLES}
  GLSLANG_LIBRARIES
)

mark_as_advanced(
  GLSLANG_HAS_REVISION_H
  ${GLSLANG_VARIABLES}
  GLSLANG_LIBRARIES
)

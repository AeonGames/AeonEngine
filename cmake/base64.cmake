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

# Function to add a custom target for decoding base64 files using aeontool
# Usage: add_b64_decode_target(target_name input_file1 [input_file2 ...])
# The output files will have .b64 removed from the input file names
function(add_b64_decode_target target_name)
  set(input_files ${ARGN})
  set(output_files "")
  set(decode_commands "")
  set(output_dirs "")
  
  foreach(input_file ${input_files})
    # Remove .b64 extension from the input file to get the output file
    get_filename_component(file_dir "${input_file}" DIRECTORY)
    get_filename_component(file_name "${input_file}" NAME)
    string(REGEX REPLACE "\\.b64$" "" output_name "${file_name}")
    set(output_file "${file_dir}/${output_name}")
    
    list(APPEND output_files "${output_file}")
    list(APPEND output_dirs "${file_dir}")
    list(APPEND decode_commands
      COMMAND ${CMAKE_COMMAND} -E echo "Decoding: ${input_file}"
      COMMAND $<TARGET_FILE:aeontool> base64 decode -i "${input_file}" -o "${output_file}"
    )
  endforeach()
  
  # Remove duplicate directories
  list(REMOVE_DUPLICATES output_dirs)
  
  # Create directory commands
  set(mkdir_commands "")
  foreach(dir ${output_dirs})
    list(APPEND mkdir_commands COMMAND ${CMAKE_COMMAND} -E make_directory "${dir}")
  endforeach()
  
  add_custom_target(
    ${target_name}
    ${mkdir_commands}
    ${decode_commands}
    DEPENDS aeontool ${input_files}
    BYPRODUCTS ${output_files}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Decoding base64 files to .${extension} format"
  )
endfunction()

# Function to add a custom target for encoding files to base64 using aeontool
# Usage: add_b64_encode_target(target_name extension input_file1 [input_file2 ...])
# The output files will have the same path as input files but with the new extension
function(add_b64_encode_target target_name extension)
  set(input_files ${ARGN})
  set(output_files "")
  set(encode_commands "")
  set(output_dirs "")
  
  foreach(input_file ${input_files})
    # Get the file path without extension and replace with new extension
    get_filename_component(file_dir "${input_file}" DIRECTORY)
    get_filename_component(file_name "${input_file}" NAME_WE)
    set(output_file "${file_dir}/${file_name}.${extension}")
    
    list(APPEND output_files "${output_file}")
    list(APPEND output_dirs "${file_dir}")
    list(APPEND encode_commands
      COMMAND ${CMAKE_COMMAND} -E echo "Encoding: ${input_file}"
      COMMAND $<TARGET_FILE:aeontool> base64 encode -i "${input_file}" -o "${output_file}"
    )
  endforeach()
  
  # Remove duplicate directories
  list(REMOVE_DUPLICATES output_dirs)
  
  # Create directory commands
  set(mkdir_commands "")
  foreach(dir ${output_dirs})
    list(APPEND mkdir_commands COMMAND ${CMAKE_COMMAND} -E make_directory "${dir}")
  endforeach()
  
  add_custom_target(
    ${target_name}
    ${mkdir_commands}
    ${encode_commands}
    DEPENDS aeontool ${input_files}
    BYPRODUCTS ${output_files}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Encoding files to base64 .${extension} format"
  )
endfunction()

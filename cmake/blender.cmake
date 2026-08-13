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

# Blender discovery and asset cooking.
#
# Blender is an optional dependency: when it is missing the configure step only
# prints a status message and the asset targets below are not generated.

# Run "<executable> --version" and return the parsed version, or an empty string
# when the executable cannot be run or does not identify itself as Blender.
function(aeon_blender_version executable output_variable)
  execute_process(COMMAND "${executable}" --version
                  OUTPUT_VARIABLE version_output
                  ERROR_QUIET
                  RESULT_VARIABLE version_result)
  if(NOT version_result EQUAL 0)
    set(${output_variable} "" PARENT_SCOPE)
    return()
  endif()
  if(version_output MATCHES "Blender[ \t]+([0-9]+\\.[0-9]+(\\.[0-9]+)?)")
    set(${output_variable} "${CMAKE_MATCH_1}" PARENT_SCOPE)
  else()
    set(${output_variable} "" PARENT_SCOPE)
  endif()
endfunction()

# Locate the newest installed Blender and cache it as BLENDER_EXECUTABLE.
#
# Blender ships one versioned directory per release rather than upgrading in
# place, so several copies commonly coexist; every candidate is asked for its
# own version and the highest one wins. A user supplied -DBLENDER_EXECUTABLE
# short-circuits the search but is still validated.
function(aeon_find_blender)
  if(BLENDER_EXECUTABLE)
    aeon_blender_version("${BLENDER_EXECUTABLE}" version)
    if(version)
      set(BLENDER_VERSION "${version}" CACHE INTERNAL "Blender version")
      message(STATUS "Using Blender ${version} (${BLENDER_EXECUTABLE})")
    else()
      message(STATUS "BLENDER_EXECUTABLE (${BLENDER_EXECUTABLE}) is not a usable Blender.")
      unset(BLENDER_EXECUTABLE CACHE)
    endif()
    return()
  endif()

  set(candidates)
  find_program(blender_on_path NAMES blender blender.exe)
  if(blender_on_path)
    list(APPEND candidates "${blender_on_path}")
  endif()
  unset(blender_on_path CACHE)

  if(WIN32)
    file(TO_CMAKE_PATH "$ENV{ProgramFiles}" program_files)
    file(TO_CMAKE_PATH "$ENV{ProgramFiles\(x86\)}" program_files_x86)
    file(GLOB globbed
         "${program_files}/Blender Foundation/Blender*/blender.exe"
         "${program_files_x86}/Blender Foundation/Blender*/blender.exe"
         "${program_files}/Steam/steamapps/common/Blender/blender.exe"
         "${program_files_x86}/Steam/steamapps/common/Blender/blender.exe")
  elseif(APPLE)
    file(GLOB globbed
         "/Applications/Blender.app/Contents/MacOS/Blender"
         "/Applications/Blender*/Blender.app/Contents/MacOS/Blender"
         "$ENV{HOME}/Applications/Blender.app/Contents/MacOS/Blender"
         "$ENV{HOME}/Applications/Blender*/Blender.app/Contents/MacOS/Blender")
  else()
    file(GLOB globbed
         "/usr/bin/blender"
         "/usr/local/bin/blender"
         "/opt/blender*/blender"
         "$ENV{HOME}/.local/bin/blender"
         "/var/lib/flatpak/exports/bin/org.blender.Blender")
  endif()
  list(APPEND candidates ${globbed})
  list(REMOVE_DUPLICATES candidates)

  set(newest_executable)
  set(newest_version)
  foreach(candidate IN LISTS candidates)
    aeon_blender_version("${candidate}" version)
    if(version AND (NOT newest_version OR version VERSION_GREATER newest_version))
      set(newest_executable "${candidate}")
      set(newest_version "${version}")
    endif()
  endforeach()

  if(NOT newest_executable)
    message(STATUS "Blender not found: the Blender asset targets will not be "
                   "generated. Set -DBLENDER_EXECUTABLE=<path> to override.")
    return()
  endif()

  # FORCE because a -NOTFOUND left in the cache by an earlier configure would
  # otherwise make these writes silent no-ops.
  set(BLENDER_EXECUTABLE "${newest_executable}" CACHE FILEPATH "Blender executable" FORCE)
  set(BLENDER_VERSION "${newest_version}" CACHE INTERNAL "Blender version")
  message(STATUS "Found Blender ${newest_version} (${newest_executable})")
endfunction()

# Ask Blender for the interpreter it bundles. Guessing the path from the
# install layout is unreliable: it differs across platforms and Blender may
# also be built against the system Python.
function(aeon_find_blender_python)
  if(BLENDER_Python3_EXECUTABLE)
    return()
  endif()
  execute_process(COMMAND "${BLENDER_EXECUTABLE}" --background --factory-startup
                          -noaudio --python-expr
                          "import sys;print('AEON_PYTHON:'+sys.executable)"
                  OUTPUT_VARIABLE python_output
                  ERROR_QUIET
                  RESULT_VARIABLE python_result)
  if(python_result EQUAL 0 AND python_output MATCHES "AEON_PYTHON:([^\r\n]+)")
    file(TO_CMAKE_PATH "${CMAKE_MATCH_1}" python_executable)
    set(BLENDER_Python3_EXECUTABLE "${python_executable}"
        CACHE FILEPATH "Python interpreter bundled with Blender" FORCE)
  endif()
endfunction()

aeon_find_blender()

if(BLENDER_EXECUTABLE)
  aeon_find_blender_python()
endif()

if(BLENDER_EXECUTABLE AND NOT BLENDER_Python3_EXECUTABLE)
  message(STATUS "Blender's Python interpreter could not be determined: the "
                 "Blender asset targets will not be generated.")
endif()

if(BLENDER_EXECUTABLE AND BLENDER_Python3_EXECUTABLE)
  set(BLENDER_VENV "${CMAKE_BINARY_DIR}/blender-venv")

  # The exporters import the generated protobuf modules, which in turn need the
  # protobuf runtime Blender does not bundle. Rather than modifying the Blender
  # installation, build a virtual environment out of its own interpreter (so the
  # ABI matches) and let export_asset.py put that site-packages on sys.path.
  # The script is a no-op once the environment matches the generated code.
  add_custom_target(
    blender-python-venv
    COMMAND ${CMAKE_COMMAND}
            -DAEON_SOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DBLENDER_VENV=${BLENDER_VENV}
            "-DBLENDER_Python3_EXECUTABLE=${BLENDER_Python3_EXECUTABLE}"
            -P ${CMAKE_SOURCE_DIR}/cmake/BlenderPythonVenv.cmake
    COMMENT "Checking the Blender Python virtual environment"
    VERBATIM)
  add_dependencies(blender-python-venv generate-python-protobuf-source)
endif()

if(BLENDER_EXECUTABLE)
  # Right now this is just a shortcut to running Blender, it was supposed to set
  # everything up so exporters were registered and ready to run without making
  # changes to the Blender configuration, but that idea didn't work out in the
  # end. The target is still useful and I may expand on it later on, so it stays
  # for now.
  add_custom_target(
    run-blender
    COMMAND "${BLENDER_EXECUTABLE}"
    DEPENDS generate-python-protobuf-source
    SOURCES ${CMAKE_SOURCE_DIR}/tools/blender/addons/io_mesh_msh/export.py
            ${CMAKE_SOURCE_DIR}/tools/blender/addons/io_skeleton_skl/export.py
            ${CMAKE_SOURCE_DIR}/tools/blender/addons/io_animation_anm/export.py
    COMMENT "Running Blender")
endif()

# Cook a .blend into engine assets with a headless Blender run.
#
# add_blender_asset(<name>
#                   BLEND <file>
#                   DESTINATION <directory>
#                   RESOURCE_PREFIX <prefix>
#                   [MODEL_NAME <name>]
#                   [TEXTURES <file>...])
#
# BLEND, DESTINATION and TEXTURES are relative to the repository root;
# RESOURCE_PREFIX is the path the generated references are prefixed with,
# relative to the game resource root. The target is never part of ALL.
function(add_blender_asset name)
  cmake_parse_arguments(ASSET "" "BLEND;DESTINATION;RESOURCE_PREFIX;MODEL_NAME" "TEXTURES" ${ARGN})
  if(NOT ASSET_BLEND OR NOT ASSET_DESTINATION OR NOT ASSET_RESOURCE_PREFIX)
    message(FATAL_ERROR "add_blender_asset(${name}) requires BLEND, DESTINATION and RESOURCE_PREFIX")
  endif()
  if(NOT ASSET_MODEL_NAME)
    set(ASSET_MODEL_NAME "${name}")
  endif()

  if(NOT TARGET blender-python-venv)
    message(STATUS "Skipping the ${name} asset target (Blender unavailable).")
    return()
  endif()

  set(texture_command)
  if(ASSET_TEXTURES)
    set(texture_sources)
    foreach(texture IN LISTS ASSET_TEXTURES)
      list(APPEND texture_sources "${CMAKE_SOURCE_DIR}/${texture}")
    endforeach()
    # Copied after the export so the sources under assets/ stay authoritative
    # for textures that also exist loose on disk.
    set(texture_command
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${texture_sources}
                "${CMAKE_SOURCE_DIR}/${ASSET_DESTINATION}/textures/")
  endif()

  add_custom_target(
    ${name}
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${CMAKE_SOURCE_DIR}/${ASSET_DESTINATION}/textures"
    COMMAND ${CMAKE_COMMAND} -E env
            "BLENDER_USER_SCRIPTS=${CMAKE_SOURCE_DIR}/tools/blender"
            "${BLENDER_EXECUTABLE}" --background --factory-startup -noaudio
            "${CMAKE_SOURCE_DIR}/${ASSET_BLEND}"
            --python "${CMAKE_SOURCE_DIR}/tools/blender/export_asset.py"
            --
            --out "${CMAKE_SOURCE_DIR}/${ASSET_DESTINATION}"
            --prefix "${ASSET_RESOURCE_PREFIX}"
            --name "${ASSET_MODEL_NAME}"
            --venv "${BLENDER_VENV}"
    ${texture_command}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    SOURCES ${CMAKE_SOURCE_DIR}/${ASSET_BLEND}
            ${CMAKE_SOURCE_DIR}/tools/blender/export_asset.py
    COMMENT "Cooking ${name} assets with Blender"
    USES_TERMINAL
    VERBATIM)

  add_dependencies(${name} generate-python-protobuf-source blender-python-venv)
endfunction()

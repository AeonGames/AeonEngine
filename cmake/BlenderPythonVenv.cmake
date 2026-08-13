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

# Provision the virtual environment the Blender exporters import from.
#
# Run with -P. Expects BLENDER_VENV, BLENDER_Python3_EXECUTABLE and
# AEON_SOURCE_DIR to be defined on the command line.
#
# The protobuf runtime has to match the generated code, and protoc stamps the
# version it generated for into every module, so that stamp is the single
# source of truth instead of a pin that silently drifts.

set(gencode "${AEON_SOURCE_DIR}/tools/blender/modules/mesh_pb2.py")
if(NOT EXISTS "${gencode}")
  message(FATAL_ERROR "${gencode} is missing; build generate-python-protobuf-source first.")
endif()

file(STRINGS "${gencode}" version_line REGEX "^# Protobuf Python Version: ")
if(NOT version_line MATCHES "([0-9]+\\.[0-9]+\\.[0-9]+)")
  message(FATAL_ERROR "Could not read the protobuf runtime version out of ${gencode}.")
endif()
set(required_version "${CMAKE_MATCH_1}")

set(marker "${BLENDER_VENV}/aeon-protobuf-version.txt")
if(EXISTS "${marker}")
  file(READ "${marker}" installed_version)
  string(STRIP "${installed_version}" installed_version)
  if(installed_version STREQUAL required_version)
    return()
  endif()
endif()

if(WIN32)
  set(venv_python "${BLENDER_VENV}/Scripts/python.exe")
else()
  set(venv_python "${BLENDER_VENV}/bin/python")
endif()

message(STATUS "Provisioning ${BLENDER_VENV} with protobuf ${required_version}")
file(REMOVE_RECURSE "${BLENDER_VENV}")
execute_process(COMMAND "${BLENDER_Python3_EXECUTABLE}" -m venv "${BLENDER_VENV}"
                COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${venv_python}" -m pip install --disable-pip-version-check
                        "protobuf==${required_version}"
                COMMAND_ERROR_IS_FATAL ANY)
file(WRITE "${marker}" "${required_version}\n")

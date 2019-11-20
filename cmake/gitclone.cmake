# Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba
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

find_package(Git REQUIRED)
function(gitclone repo)
  cmake_parse_arguments(CLONE "" "PATH TAG COMMIT" "" ${ARGN})
  if(NOT CLONE_TAG)
    set(CLONE_TAG "master")
  endif()
  if(NOT CLONE_PATH)
    get_filename_component(CLONE_PATH ${repo} NAME_WE)
  endif()
  if(NOT IS_DIRECTORY "${CLONE_PATH}")
      message(STATUS "Cloning ${repo}, please wait")
      execute_process(COMMAND ${GIT_EXECUTABLE} clone -b ${CLONE_TAG} ${repo} ${CLONE_PATH} RESULT_VARIABLE git_result OUTPUT_VARIABLE git_output ERROR_VARIABLE git_output)
      if(NOT git_result EQUAL 0)
          MESSAGE(FATAL_ERROR "Cloning ${repo} failed.\nResult: ${git_result}\nOutput: ${git_output}")
      endif(NOT git_result EQUAL 0)
    if(CLONE_COMMIT)
        execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${CLONE_COMMIT} WORKING_DIRECTORY ${CLONE_PATH} RESULT_VARIABLE git_result OUTPUT_VARIABLE git_output ERROR_VARIABLE git_output)
        if(NOT git_result EQUAL 0)
            MESSAGE(FATAL_ERROR "Cloning ${repo} failed.\nResult: ${git_result}\nOutput: ${git_output}")
        endif(NOT git_result EQUAL 0)
    endif()
  endif(NOT IS_DIRECTORY "${CLONE_PATH}")
endfunction()

# Copyright (C) 2019,2025,2026 Rodrigo Jose Hernandez Cordoba
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
include(CMakeParseArguments)
function(gitclone)
	cmake_parse_arguments(
			ARG # prefix of output variables
			"" # list of names of the boolean arguments (only defined ones will be true)
			"REPO;PATH;TAG" # list of names of mono-valued arguments
			"PATCHES" # list of names of multi-valued arguments (output variables are lists)
			${ARGN} # arguments of the function to parse, here we take the all original ones
		)
	if(NOT IS_DIRECTORY "${ARG_PATH}")
		message(STATUS "Cloning ${ARG_REPO}, please wait")
		execute_process(COMMAND ${GIT_EXECUTABLE} clone --depth 1 ${ARG_REPO} ${ARG_PATH} WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" RESULT_VARIABLE git_result OUTPUT_VARIABLE git_output ERROR_VARIABLE git_output)
		if(NOT git_result EQUAL 0)
			MESSAGE(FATAL_ERROR "Cloning ${ARG_REPO} failed.\nResult: ${git_result}\nOutput: ${git_output}")
		endif(NOT git_result EQUAL 0)

		execute_process(COMMAND ${GIT_EXECUTABLE} fetch --depth 1 origin ${ARG_TAG} WORKING_DIRECTORY "${ARG_PATH}" RESULT_VARIABLE git_result OUTPUT_VARIABLE git_output ERROR_VARIABLE git_output)
		if(NOT git_result EQUAL 0)
			MESSAGE(FATAL_ERROR "Fetching ${ARG_TAG} failed.\nResult: ${git_result}\nOutput: ${git_output}")
		endif(NOT git_result EQUAL 0)

		execute_process(COMMAND ${GIT_EXECUTABLE} checkout FETCH_HEAD WORKING_DIRECTORY "${ARG_PATH}" RESULT_VARIABLE git_result OUTPUT_VARIABLE git_output ERROR_VARIABLE git_output)
		if(NOT git_result EQUAL 0)
			MESSAGE(FATAL_ERROR "Checkout ${FETCH_HEAD} failed.\nResult: ${git_result}\nOutput: ${git_output}")
		endif(NOT git_result EQUAL 0)
		foreach(PATCH ${ARG_PATCHES})
			message(STATUS "Applying patch ${PATCH}")
			execute_process(COMMAND ${GIT_EXECUTABLE} apply ${PATCH} WORKING_DIRECTORY "${ARG_PATH}" RESULT_VARIABLE git_result OUTPUT_VARIABLE git_output ERROR_VARIABLE git_output)
			if(NOT git_result EQUAL 0)
				MESSAGE(FATAL_ERROR "Failed to apply ${PATCH}.\nResult: ${git_result}\nOutput: ${git_output}")
			endif(NOT git_result EQUAL 0)
			endforeach()
	endif(NOT IS_DIRECTORY "${ARG_PATH}")
endfunction(gitclone)

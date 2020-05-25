# Copyright (C) 2020 Rodrigo Jose Hernandez Cordoba
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

# These should match versions releases at https://www.electronjs.org/
set(ELECTRON_RUNTIME_VERSION "v8.2.5")
set(NODE_RUNTIME_VERSION "v12.13.0")

if(WIN32)
    set(SUFFIX "win32-x64")
elseif(UNIX)
    set(SUFFIX "linux-x64")
endif()

function(download url)
	get_filename_component(filename ${url} NAME)
	if(ARGV1)
		set(location ${ARGV1})
	else()
		set(location ${CMAKE_BINARY_DIR})
	endif()
	if(NOT EXISTS "${location}/${filename}")
		message(STATUS "Downloading ${url}")
		file(DOWNLOAD "${url}" "${location}/${filename}" STATUS download_status SHOW_PROGRESS)
		list(GET download_status 0 download_status_code)
		list(GET download_status 1 download_status_desc)
		if(download_status_code)
			file(REMOVE "${location}/${filename}")
			message(FATAL_ERROR "Download failed\nError Code: ${download_status_code}\nMessage: ${download_status_desc}")
		endif()
		message(STATUS "Done downloading ${filename}")
	endif()
endfunction(download url)

function(decompress_tar filename)
    get_filename_component(directory ${filename} NAME)
    string(REPLACE "-headers.tar.xz" "" directory ${directory})

    if(NOT IS_DIRECTORY "${CMAKE_BINARY_DIR}/${directory}")
		message(STATUS "Extracting ${filename}")
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzvf "${CMAKE_BINARY_DIR}/${filename}" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" RESULT_VARIABLE extract_result)
		if(extract_result)
			message(FATAL_ERROR "Extracting Failed with error code: ${extract_result}")
		endif()
	endif()
endfunction(decompress_tar filename)

function(decompress_electron)
	set(ELECTRON_BINARY_DIR "${CMAKE_BINARY_DIR}/electron-${ELECTRON_RUNTIME_VERSION}-${SUFFIX}")
	if(NOT IS_DIRECTORY "${ELECTRON_BINARY_DIR}")
		file(MAKE_DIRECTORY "${ELECTRON_BINARY_DIR}")
		message(STATUS "Extracting ${ELECTRON_BINARY_DIR}.zip")
		execute_process(
			COMMAND 
				${CMAKE_COMMAND} -E tar xzvf "${ELECTRON_BINARY_DIR}.zip" WORKING_DIRECTORY "${ELECTRON_BINARY_DIR}" RESULT_VARIABLE extract_result)
		if(extract_result)
			message(FATAL_ERROR "Extracting Failed with error code: ${extract_result}")
		endif()
		file(MAKE_DIRECTORY "${ELECTRON_BINARY_DIR}/resources/app")
		file(REMOVE "${ELECTRON_BINARY_DIR}/resources/default_app.asar")
		set(ELECTRON_BINARY_DIR "${ELECTRON_BINARY_DIR}" CACHE PATH "Electron binary location.")
		set(ELECTRON_EXECUTABLE "${ELECTRON_BINARY_DIR}/electron${CMAKE_EXECUTABLE_SUFFIX}" CACHE PATH "Electron executable.")
	endif()
endfunction(decompress_electron)

download(https://github.com/electron/electron/releases/download/${ELECTRON_RUNTIME_VERSION}/electron-${ELECTRON_RUNTIME_VERSION}-${SUFFIX}.zip)
download(https://nodejs.org/download/release/${NODE_RUNTIME_VERSION}/node-${NODE_RUNTIME_VERSION}-headers.tar.xz)

decompress_electron()
decompress_tar(node-${NODE_RUNTIME_VERSION}-headers.tar.xz)
set(NODE_INCLUDE_DIR ${CMAKE_BINARY_DIR}/node-${NODE_RUNTIME_VERSION}/include/node CACHE PATH "Node headers location")

if(NOT IS_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

if(WIN32 AND MSVC)
    download(https://nodejs.org/download/release/${NODE_RUNTIME_VERSION}/win-x64/node.lib "${CMAKE_BINARY_DIR}/lib")
    set(NODE_LIBRARY ${CMAKE_BINARY_DIR}/node.lib CACHE PATH "Node headers location")
elseif(WIN32 AND MINGW)
	find_program(GENDEF gendef HINTS ENV MINGW_PREFIX MSYS2_PATH)
	find_program(DLLTOOL dlltool HINTS ENV MINGW_PREFIX MSYS2_PATH)
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/lib/libnode.a")
		message(STATUS "Generating electron.def")
		execute_process(
            COMMAND ${GENDEF} ${ELECTRON_EXECUTABLE} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} ERROR_QUIET)
		message(STATUS "Generating libnode.a")
		execute_process(
            COMMAND ${DLLTOOL} --dllname node.exe --def electron.def --output-delaylib ${CMAKE_BINARY_DIR}/lib/libnode.a WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif()
    set(NODE_LIBRARY ${CMAKE_BINARY_DIR}/lib/libnode.a CACHE PATH "Node library location")
endif()

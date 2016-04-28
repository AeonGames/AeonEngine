# Copyright 2016 Rodrigo Jose Hernandez Cordoba
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

cmake_minimum_required(VERSION 2.8.12)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	if(POLICY CMP0020)
		cmake_policy(SET CMP0020 NEW)
	endif(POLICY CMP0020)
endif(COMMAND cmake_policy)

project(AeonEngine)

set(RUNTIME_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/runtime" CACHE PATH "Where to find the Aeon Games Runtime instalation.")
list(APPEND CMAKE_PREFIX_PATH "${RUNTIME_INSTALL_PREFIX}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${RUNTIME_INSTALL_PREFIX}/cmake")
list(APPEND CMAKE_MODULE_PATH "${RUNTIME_INSTALL_PREFIX}/lib/cmake/protobuf")

if(CMAKE_COMPILER_IS_GNUCXX)
	list(APPEND CMAKE_CXX_FLAGS "-std=gnu++14")
endif()

find_package(Vulkan)
find_package(Protobuf)

add_subdirectory(engine)
add_subdirectory(application)

find_program (ASTYLE_EXECUTABLE astyle)

set(ASTYLE_OPTIONS "--options=${CMAKE_SOURCE_DIR}/astylerc")
configure_file("${CMAKE_SOURCE_DIR}/cmake/pre-commit.in"
	"${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit")

add_custom_target(format-code 
				${ASTYLE_EXECUTABLE} --options=${CMAKE_SOURCE_DIR}/astylerc ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
				COMMENT "Formatting Code."
)

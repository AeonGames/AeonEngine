# Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba
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

include_directories(${GTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)
set(TESTED_SRCS
    ${CMAKE_SOURCE_DIR}/include/aeongames/DependencyMap.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Tree.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/CRC.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Node.h
)
set(TEST_SRCS
    Main.cpp
    DependencyMapTests.cpp
    CRCTests.cpp
    NodeTests.cpp
    TreeTests.cpp
)

source_group("Tested" FILES ${TESTED_SRCS})
source_group("Tests" FILES ${TEST_SRCS})

add_executable(unit-tests ${TEST_SRCS} ${TESTED_SRCS})

if(GTEST_LIBRARY AND GTEST_LIBRARY_DEBUG)
 set(GTEST_LIBRARIES optimized ${GTEST_LIBRARY} debug ${GTEST_LIBRARY_DEBUG})
else()
 if(GTEST_LIBRARY_DEBUG)
  set(GTEST_LIBRARIES ${GTEST_LIBRARY_DEBUG})
 endif()
 if(GTEST_LIBRARY)
  set(GTEST_LIBRARIES ${GTEST_LIBRARY})
 endif()
endif()

target_link_libraries(unit-tests AeonEngine ${GTEST_LIBRARIES})
set_target_properties(unit-tests PROPERTIES
    COMPILE_FLAGS "-D_CRT_SECURE_NO_WARNINGS")
add_test(NAME unit-tests COMMAND unit-tests)

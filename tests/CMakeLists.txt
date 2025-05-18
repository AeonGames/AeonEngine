# Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba
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

include_directories(${GTEST_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_SOURCE_DIR}/engine/include
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${PROTOBUF_INCLUDE_DIR}
                    ${CMAKE_BINARY_DIR}/engine)
set(TESTED_SRCS
    ${CMAKE_SOURCE_DIR}/engine/include/Decoder.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/DependencyMap.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Scene.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/CRC.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Quaternion.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Package.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/ResourceCache.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Archive.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/Container.h
    ${CMAKE_SOURCE_DIR}/include/aeongames/UniqueAnyPtr.h)
set(TEST_SRCS
    Main.cpp
    ArchiveTests.cpp
    ContainerTests.cpp
    DecoderTests.cpp
    DependencyMapTests.cpp
    FlyWeightTests.cpp
    CRCTests.cpp
    SceneTests.cpp
    MatrixTests.cpp
    TransformTests.cpp
    QuaternionTests.cpp
    PackageTests.cpp
    ResourceCacheTests.cpp
    ResourceIdTests.cpp
    UniqueAnyPtrTests.cpp)

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

if(GMOCK_LIBRARY AND GMOCK_LIBRARY_DEBUG)
  set(GMOCK_LIBRARIES optimized ${GMOCK_LIBRARY} debug ${GMOCK_LIBRARY_DEBUG})
else()
  if(GMOCK_LIBRARY_DEBUG)
    set(GMOCK_LIBRARIES ${GMOCK_LIBRARY_DEBUG})
  endif()
  if(GMOCK_LIBRARY)
    set(GMOCK_LIBRARIES ${GMOCK_LIBRARY})
  endif()
endif()

target_link_libraries(unit-tests
                      AeonEngine
                      Threads::Threads
                      ${GTEST_LIBRARIES}
                      ${GMOCK_LIBRARIES})
if(MSVC)
  # VCPKG compiles GTest/GMock DLLs.
  set_target_properties(
    unit-tests
    PROPERTIES
      COMPILE_FLAGS
      "-DSOURCE_PATH=\"\\\"${CMAKE_SOURCE_DIR}\\\"\" -DGTEST_LINKED_AS_SHARED_LIBRARY -D_CRT_SECURE_NO_WARNINGS -wd4251 -wd4275"
    )
else()
  set_target_properties(
    unit-tests
    PROPERTIES COMPILE_FLAGS "-DSOURCE_PATH=\"\\\"${CMAKE_SOURCE_DIR}\\\"\"")
endif()
add_test(NAME unit-tests COMMAND unit-tests)

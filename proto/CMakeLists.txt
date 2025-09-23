# Copyright (C) 2024,2025 Rodrigo Jose Hernandez Cordoba
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

include_directories(
                    ${Protobuf_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/include
)

add_library(ProtoBufClasses SHARED
  configuration.proto
  vector2.proto
  vector3.proto
  vector4.proto
  quaternion.proto
  property.proto
  sampler.proto
  mesh.proto
  skeleton.proto
  pipeline.proto
  material.proto
  model.proto
  modelcontroller.proto
  transform.proto
  scene.proto
  reference.proto
  framebuffer.proto
  matrix4x4.proto
  animation.proto
  collision.proto
  ${CMAKE_SOURCE_DIR}/include/aeongames/ProtoBufClasses.hpp
)

protobuf_generate(TARGET ProtoBufClasses
                      EXPORT_MACRO DLL_PROTOBUF
                      IMPORT_DIRS ${CMAKE_SOURCE_DIR}/proto
)

target_link_libraries(ProtoBufClasses ${Protobuf_LIBRARIES})

if(MSVC)
  set_target_properties(
    ProtoBufClasses
    PROPERTIES COMPILE_FLAGS
               "-FIaeongames/ProtoBufClasses.hpp -WX -D_CRT_SECURE_NO_WARNINGS -wd4251")
elseif(MINGW OR MSYS OR UNIX)
  set_target_properties(
    ProtoBufClasses
    PROPERTIES
      COMPILE_FLAGS
      "-include aeongames/ProtoBufClasses.hpp")
endif()

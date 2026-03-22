/*
Copyright (C) 2018,2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef AEONGAMES_PROTOBUFUTILS_H
#define AEONGAMES_PROTOBUFUTILS_H
#include <cstdint>
#include <string>
#include "aeongames/Platform.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Quaternion.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Property.hpp"
#include "aeongames/Material.hpp"
namespace AeonGames
{
    class ReferenceMsg;
    class PipelineMsg;
    class Vector3Msg;
    class QuaternionMsg;
    class TransformMsg;
    class ComponentPropertyMsg;
    class PipelineMsg;
    /** @brief Retrieve the identifier from a ReferenceMsg.
     *  @param reference_buffer The ReferenceMsg to extract the id from.
     *  @return The 32-bit identifier stored in the reference message.
     */
    DLL uint32_t GetReferenceMsgId ( const ReferenceMsg& reference_buffer );
    /** @brief Get the packed attribute flags from a pipeline message.
     *  @param aPipelineMsg The pipeline message to query.
     *  @return Bitmask of vertex attributes used by the pipeline.
     */
    DLL uint32_t GetAttributes ( const PipelineMsg& aPipelineMsg );
    /** @brief Generate GLSL attribute declarations from a pipeline message.
     *  @param aPipelineMsg The pipeline message to query.
     *  @return A string containing GLSL attribute declarations.
     */
    DLL std::string GetAttributesGLSL ( const PipelineMsg& aPipelineMsg );
    /** @brief Generate GLSL property/uniform declarations from a pipeline message.
     *  @param aPipelineMsg The pipeline message to query.
     *  @return A string containing GLSL uniform declarations.
     */
    DLL std::string GetPropertiesGLSL ( const PipelineMsg& aPipelineMsg );
    /** @brief Convert a Vector3Msg protobuf message to a Vector3.
     *  @param aVector3 The protobuf message to convert.
     *  @return The corresponding Vector3 value.
     */
    Vector3 GetVector3 ( const Vector3Msg& aVector3 );
    /** @brief Convert a QuaternionMsg protobuf message to a Quaternion.
     *  @param aQuaternion The protobuf message to convert.
     *  @return The corresponding Quaternion value.
     */
    Quaternion GetQuaternion ( const QuaternionMsg& aQuaternion );
    /** @brief Convert a TransformMsg protobuf message to a Transform.
     *  @param aTransform The protobuf message to convert.
     *  @return The corresponding Transform value.
     */
    Transform GetTransform ( const TransformMsg& aTransform );
    /** @brief Convert a ComponentPropertyMsg protobuf message to a Property.
     *  @param aComponentPropertyMsg The protobuf message to convert.
     *  @return The corresponding Property value.
     */
    Property GetProperty ( const ComponentPropertyMsg& aComponentPropertyMsg );
    /** @brief Calculate the uniform buffer size required by a pipeline.
     *  @param aPipelineMsg The pipeline message to query.
     *  @return Size in bytes of the uniform buffer.
     */
    size_t GetUniformBufferSize ( const PipelineMsg& aPipelineMsg );
    /** @brief Convert a PropertyMsg to a Material uniform key-value pair.
     *  @param aProperty The property message to convert.
     *  @return A key-value pair suitable for Material uniform data.
     */
    DLL Material::UniformKeyValue PropertyToKeyValue ( const PropertyMsg& aProperty );
}
#endif

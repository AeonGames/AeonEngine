/*
Copyright (C) 2018,2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
    DLL uint32_t GetReferenceMsgId ( const ReferenceMsg& reference_buffer );
    DLL uint32_t GetAttributes ( const PipelineMsg& aPipelineMsg );
    DLL std::string GetAttributesGLSL ( const PipelineMsg& aPipelineMsg );
    DLL std::string GetPropertiesGLSL ( const PipelineMsg& aPipelineMsg );
    Vector3 GetVector3 ( const Vector3Msg& aVector3 );
    Quaternion GetQuaternion ( const QuaternionMsg& aQuaternion );
    Transform GetTransform ( const TransformMsg& aTransform );
    Property GetProperty ( const ComponentPropertyMsg& aComponentPropertyMsg );
    size_t GetUniformBufferSize ( const PipelineMsg& aPipelineMsg );
    DLL Material::UniformKeyValue PropertyToKeyValue ( const PropertyMsg& aProperty );
}
#endif

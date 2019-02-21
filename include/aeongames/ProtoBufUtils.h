/*
Copyright (C) 2018,2019 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Platform.h"
#include "aeongames/Vector3.h"
#include "aeongames/Quaternion.h"
#include "aeongames/Transform.h"
#include "aeongames/Property.h"
namespace AeonGames
{
    class ReferenceBuffer;
    class PipelineBuffer;
    class Vector3Buffer;
    class QuaternionBuffer;
    class TransformBuffer;
    class ComponentPropertyBuffer;
    DLL uint32_t GetReferenceBufferId ( const ReferenceBuffer& reference_buffer );
    DLL uint32_t GetAttributes ( const PipelineBuffer& aPipelineBuffer );
    DLL std::string GetAttributesGLSL ( const PipelineBuffer& aPipelineBuffer );
    DLL std::string GetPropertiesGLSL ( const PipelineBuffer& aPipelineBuffer );
    Vector3 GetVector3 ( const Vector3Buffer& aVector3 );
    Quaternion GetQuaternion ( const QuaternionBuffer& aQuaternion );
    Transform GetTransform ( const TransformBuffer& aTransform );
    Property GetProperty ( const ComponentPropertyBuffer& aComponentPropertyBuffer );
}
#endif

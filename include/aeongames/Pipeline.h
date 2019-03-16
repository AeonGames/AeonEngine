/*
Copyright (C) 2017-2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PIPELINE_H
#define AEONGAMES_PIPELINE_H

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <regex>
#include "aeongames/Platform.h"
#include "aeongames/Material.h"

namespace AeonGames
{
    class MaterialBuffer;
    class PipelineBuffer;

    enum AttributeBits
    {
        VertexPositionBit = 0x1,
        VertexNormalBit = 0x2,
        VertexTangentBit = 0x4,
        VertexBitangentBit = 0x8,
        VertexUVBit = 0x10,
        VertexWeightIndicesBit = 0x20,
        VertexWeightsBit = 0x40,
        VertexColorBit = 0x80,
        VertexAllBits = VertexPositionBit |
                        VertexNormalBit |
                        VertexTangentBit |
                        VertexBitangentBit |
                        VertexUVBit |
                        VertexWeightIndicesBit |
                        VertexWeightsBit |
                        VertexColorBit
    };

    enum AttributeFormat
    {
        Vector2Float,
        Vector3Float,
        Vector4Byte,
        Vector4ByteNormalized,
    };

    class Pipeline
    {
    public:
        DLL virtual ~Pipeline() = 0;
        virtual void Load ( const PipelineBuffer& aPipelineBuffer ) = 0;
        virtual void Unload() = 0;
        virtual const Material& GetDefaultMaterial() const = 0;
        // Non Virtual
        DLL void Load ( const std::string& aFilename );
        DLL void Load ( uint32_t aId );
        DLL void Load ( const void* aBuffer, size_t aBufferSize );
    };
}
#endif

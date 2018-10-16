/*
Copyright (C) 2017-2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Platform.h"
#include "aeongames/Material.h"

namespace AeonGames
{
    class PipelineBuffer;
#if 1
    class Pipeline
    {
    public:
        DLL virtual ~Pipeline() = 0;
        virtual void Load ( const std::string& aFilename ) = 0;
        virtual void Load ( uint32_t aId ) = 0;
        virtual void Load ( const void* aBuffer, size_t aBufferSize ) = 0;
        virtual void Load ( const PipelineBuffer& aPipelineBuffer ) = 0;
        virtual void Unload() = 0;
        virtual const Material* GetDefaultMaterial() const = 0;
    };
#else
    class Pipeline
    {
    public:
        enum class Topology
        {
            POINT_LIST = 0,
            LINE_STRIP,
            LINE_LIST,
            TRIANGLE_STRIP,
            TRIANGLE_FAN,
            TRIANGLE_LIST,
            LINE_LIST_WITH_ADJACENCY,
            LINE_STRIP_WITH_ADJACENCY,
            TRIANGLE_LIST_WITH_ADJACENCY,
            TRIANGLE_STRIP_WITH_ADJACENCY,
            PATCH_LIST
        };
        enum AttributeBits
        {
            VertexPositionBit = 0x1,
            VertexNormalBit = 0x2,
            VertexTangentBit = 0x4,
            VertexBitangentBit = 0x8,
            VertexUVBit = 0x10,
            VertexWeightIndicesBit = 0x20,
            VertexWeightsBit = 0x40,
            VertexAllBits = VertexPositionBit |
                            VertexNormalBit |
                            VertexTangentBit |
                            VertexBitangentBit |
                            VertexUVBit |
                            VertexWeightIndicesBit |
                            VertexWeightsBit
        };
        enum AttributeFormat
        {
            Vector2Float,
            Vector3Float,
            Vector4Byte,
            Vector4ByteNormalized,
        };
        DLL Pipeline();
        DLL Pipeline ( uint32_t aId );
        DLL Pipeline ( const std::string& aFilename );
        DLL Pipeline ( const void* aBuffer, size_t aBufferSize );
        DLL virtual ~Pipeline();
        DLL void Load ( const std::string& aFilename );
        DLL void Load ( const void* aBuffer, size_t aBufferSize );
        DLL void Unload ();
        DLL const std::string& GetVertexShaderSource() const;
        DLL const std::string& GetFragmentShaderSource() const;
        DLL uint32_t GetAttributes() const;
        DLL uint32_t GetStride() const;
        DLL uint32_t GetLocation ( AttributeBits aAttributeBit ) const;
        DLL AttributeFormat GetFormat ( AttributeBits aAttributeBit ) const;
        DLL uint32_t GetSize ( AttributeBits aAttributeBit ) const;
        DLL uint32_t GetOffset ( AttributeBits aAttributeBit ) const;
        DLL const Material& GetDefaultMaterial() const;
        DLL const Topology GetTopology() const;
        DLL static const std::shared_ptr<Pipeline> GetPipeline ( uint32_t aId );
        DLL static const std::shared_ptr<Pipeline> GetPipeline ( const std::string& aPath );
    private:
        void Load ( const PipelineBuffer& aPipelineBuffer );
        Topology mTopology{};
        uint32_t mAttributes{};
        std::string mVertexShader{};
        std::string mFragmentShader{};
        Material mDefaultMaterial{};
    };
#endif
}
#endif

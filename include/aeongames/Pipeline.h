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
#include "Uniform.h"
#include "aeongames/Platform.h"
#include "aeongames/Material.h"

namespace AeonGames
{
    class PipelineBuffer;
    class Pipeline
    {
    public:
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
        class IRenderPipeline
        {
        public:
            DLL virtual ~IRenderPipeline() = 0;
        };
        DLL Pipeline();
        DLL Pipeline ( const std::string& aFilename );
        DLL Pipeline ( const void* aBuffer, size_t aBufferSize );
        DLL ~Pipeline();
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
        DLL void SetRenderPipeline ( std::unique_ptr<IRenderPipeline> aRenderPipeline ) const;
        DLL const IRenderPipeline* const GetRenderPipeline() const;
    private:
        void Load ( const PipelineBuffer& aPipelineBuffer );
        std::string mFilename;
        uint32_t mAttributes;
        std::string mVertexShader;
        std::string mFragmentShader;
        Material mDefaultMaterial;
        mutable std::unique_ptr<IRenderPipeline> mRenderPipeline{};
    };
}
#endif

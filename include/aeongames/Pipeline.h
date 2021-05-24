/*
Copyright (C) 2017-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include <tuple>
#include "aeongames/Platform.h"
#include "aeongames/Material.h"
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "pipeline.pb.h"
#include "property.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Resource.h"

namespace AeonGames
{
    class MaterialMsg;
    class PipelineMsg;
    enum Topology
    {
        UNDEFINED = 0,
        POINT_LIST,
        LINE_STRIP,
        LINE_LIST,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
        TRIANGLE_LIST,
        LINE_LIST_WITH_ADJACENCY,
        LINE_STRIP_WITH_ADJACENCY,
        TRIANGLE_LIST_WITH_ADJACENCY,
        TRIANGLE_STRIP_WITH_ADJACENCY,
        PATCH_LIST,
    };

    enum AttributeBits
    {
        VertexPositionBit = 0x1,
        VertexNormalBit = 0x2,
        VertexTangentBit = 0x4,
        VertexBitangentBit = 0x8,
        VertexUVBit = 0x10,
        VertexWeightIdxBit = 0x20,
        VertexWeightBit = 0x40,
        VertexColorBit = 0x80,
        VertexAllBits = VertexPositionBit |
                        VertexNormalBit |
                        VertexTangentBit |
                        VertexBitangentBit |
                        VertexUVBit |
                        VertexWeightIdxBit |
                        VertexWeightBit |
                        VertexColorBit
    };

    enum AttributeFormat
    {
        Vector2Float,
        Vector3Float,
        Vector4Byte,
        Vector4ByteNormalized,
    };

    enum UniformType
    {
        SCALAR_FLOAT,
        SCALAR_UINT,
        SCALAR_INT,
        VECTOR_FLOAT_2,
        VECTOR_FLOAT_3,
        VECTOR_FLOAT_4,
    };

    class Pipeline : public Resource<PipelineMsg, "AEONPLN"_mgk>
    {
    public:
        DLL Pipeline();
        DLL virtual ~Pipeline();
        DLL void Load ( const PipelineMsg& aPipelineMsg ) final;
        DLL void Unload() final;
        DLL Topology GetTopology() const;
        DLL const std::string& GetVertexShaderCode() const;
        DLL const std::string& GetFragmentShaderCode() const;
        DLL const std::vector<std::tuple<UniformType, std::string>>& GetUniformDescriptors() const;
        DLL const std::vector<std::string>& GetSamplerDescriptors() const;
        DLL std::string GetProperties () const;
        DLL std::string GetAttributes () const;
        DLL uint32_t GetAttributeBitmap() const;
    private:
        Topology mTopology{};
        std::string mVertexShaderCode{};
        std::string mFragmentShaderCode{};
        std::vector<std::tuple<UniformType, std::string>> mUniformDescriptors{};
        std::vector<std::string> mSamplerDescriptors{};
    };
}
#endif

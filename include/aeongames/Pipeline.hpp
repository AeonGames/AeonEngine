/*
Copyright (C) 2017-2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include <string_view>
#include <vector>
#include <array>
#include <regex>
#include <tuple>
#include "aeongames/Platform.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Resource.hpp"
#include <functional>

namespace AeonGames
{
    class PipelineMsg;
    /// @todo: This enum should be moved to a place where it makes more sence, like mesh metadata.
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

#if 0
    class MaterialMsg;

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
#endif

    enum ShaderType
    {
        VERT = 0, //  vertex shader
        FRAG,     //  fragment shader
        COMP,     //  compute shader
        TESC,     //  tessellation control shader
        TESE,     //  tessellation evaluation shader
        GEOM,     //  geometry shader
        COUNT
    };

    const std::unordered_map<ShaderType, const char*> ShaderTypeToString
    {
        { VERT, "vertex" },
        { FRAG, "fragment" },
        { COMP, "compute" },
        { TESC, "tessellation control" },
        { TESE, "tessellation evaluation" },
        { GEOM, "geometry" }
    };

    class Pipeline : public Resource
    {
    public:
        DLL Pipeline();
        DLL virtual ~Pipeline();
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        DLL void Unload() final;
#if 0
        DLL Topology GetTopology() const;
        DLL const std::string& GetVertexShaderCode() const;
        DLL const std::string& GetFragmentShaderCode() const;
        DLL const std::vector<std::tuple<UniformType, std::string >> & GetUniformDescriptors() const;
        DLL const std::vector<std::string>& GetSamplerDescriptors() const;
        DLL std::string GetProperties () const;
        DLL std::string GetAttributes () const;
        DLL uint32_t GetAttributeBitmap() const;
#endif
        DLL const std::string_view GetShaderCode ( ShaderType aType ) const;

        DLL void LoadFromPBMsg ( const PipelineMsg& aPipelineMsg );
    private:
#if 0
        Topology mTopology {};
        std::string mVertexShaderCode{};
        std::string mFragmentShaderCode{};
        std::vector<std::tuple<UniformType, std::string >> mUniformDescriptors{};
        std::vector<std::string> mSamplerDescriptors{};
#endif
        std::array<std::string, ShaderType::COUNT> mShaderCode {};
    };
}
#endif

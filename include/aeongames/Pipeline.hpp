/*
Copyright (C) 2017-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
    /** Primitive topology types for rendering.
     * @todo This enum should be moved to a place where it makes more sence, like mesh metadata.
     */
    enum Topology
    {
        UNDEFINED = 0,                  /**< Undefined topology. */
        POINT_LIST,                     /**< List of individual points. */
        LINE_STRIP,                     /**< Connected line segments. */
        LINE_LIST,                      /**< Pairs of vertices forming individual lines. */
        TRIANGLE_STRIP,                 /**< Connected triangle strip. */
        TRIANGLE_FAN,                   /**< Triangles sharing a common vertex. */
        TRIANGLE_LIST,                  /**< Independent triangles. */
        LINE_LIST_WITH_ADJACENCY,       /**< Line list with adjacency information. */
        LINE_STRIP_WITH_ADJACENCY,      /**< Line strip with adjacency information. */
        TRIANGLE_LIST_WITH_ADJACENCY,   /**< Triangle list with adjacency information. */
        TRIANGLE_STRIP_WITH_ADJACENCY,  /**< Triangle strip with adjacency information. */
        PATCH_LIST,                     /**< Patch list for tessellation. */
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

    /** Shader stage types. */
    enum ShaderType
    {
        VERT = 0, /**< Vertex shader. */
        FRAG,     /**< Fragment shader. */
        COMP,     /**< Compute shader. */
        TESC,     /**< Tessellation control shader. */
        TESE,     /**< Tessellation evaluation shader. */
        GEOM,     /**< Geometry shader. */
        COUNT     /**< Number of shader types. */
    };

    /** Map from ShaderType enum values to human-readable string names. */
    const std::unordered_map<ShaderType, const char*> ShaderTypeToString
    {
        { VERT, "vertex" },
        { FRAG, "fragment" },
        { COMP, "compute" },
        { TESC, "tessellation control" },
        { TESE, "tessellation evaluation" },
        { GEOM, "geometry" }
    };

    /** Rendering pipeline resource.
     *
     * Manages shader code and topology classification for a rendering pipeline.
     */
    class Pipeline : public Resource
    {
    public:
        static const uint32_t TOPOLOGY_CLASS_POINT{1};    /**< Point topology class bitmask. */
        static const uint32_t TOPOLOGY_CLASS_LINE{2};     /**< Line topology class bitmask. */
        static const uint32_t TOPOLOGY_CLASS_TRIANGLE{4}; /**< Triangle topology class bitmask. */
        static const uint32_t TOPOLOGY_CLASS_PATCH{8};    /**< Patch topology class bitmask. */

        /** Default constructor. */
        DLL Pipeline();
        /** Virtual destructor. */
        DLL virtual ~Pipeline();
        /** Load pipeline data from a memory buffer.
         * @param aBuffer Pointer to the buffer containing pipeline data.
         * @param aBufferSize Size of the buffer in bytes.
         */
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        /** Release all pipeline resources. */
        DLL void Unload() final;
        /** Get the topology class bitmask for this pipeline.
         * @return Bitmask indicating the topology class.
         */
        DLL uint32_t GetTopologyClass() const;
#if 0
        DLL const std::string& GetVertexShaderCode() const;
        DLL const std::string& GetFragmentShaderCode() const;
        DLL const std::vector<std::tuple<UniformType, std::string >> & GetUniformDescriptors() const;
        DLL const std::vector<std::string>& GetSamplerDescriptors() const;
        DLL std::string GetProperties () const;
        DLL std::string GetAttributes () const;
        DLL uint32_t GetAttributeBitmap() const;
#endif
        /** Get the shader source code for the given shader stage.
         * @param aType The shader stage to retrieve code for.
         * @return String view of the shader source code.
         */
        DLL const std::string_view GetShaderCode ( ShaderType aType ) const;

        /** Load pipeline configuration from a protobuf message.
         * @param aPipelineMsg The protobuf message to load from.
         */
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
        uint32_t mTopologyClass{ TOPOLOGY_CLASS_TRIANGLE };
    };
}
#endif

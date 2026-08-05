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
#include <unordered_map>
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

    /** Resource kinds exposed by generated shader-interface metadata. */
    enum class ShaderResourceType : uint8_t
    {
        Unknown,
        UniformBuffer,
        StorageBuffer,
        SampledImage,
        StorageImage,
        Sampler,
        CombinedImageSampler
    };

    /** One named shader resource and its backend-neutral set/binding address. */
    struct ShaderResource
    {
        std::string name{};
        uint32_t name_hash{0};
        uint32_t set{0};
        uint32_t binding{0};
        uint32_t count{0};
        ShaderResourceType type{ShaderResourceType::Unknown};
    };

    enum class ShaderInputScalarType : uint8_t
    {
        Unknown,
        Float,
        Int,
        UInt
    };

    /** One active vertex-stage input consumed by the compiled shader. */
    struct ShaderInput
    {
        std::string name{};
        uint32_t name_hash{0};
        uint32_t location{0};
        uint32_t components{0};
        ShaderInputScalarType scalar_type{ShaderInputScalarType::Unknown};
    };

    /** Generated interface for one graphics stage or ordered compute stage. */
    struct ShaderInterface
    {
        ShaderType stage{ShaderType::VERT};
        uint32_t compute_stage_index{0};
        std::string entry_point{"main"};
        std::array<uint32_t, 3> local_size{1, 1, 1};
        uint32_t fragment_output_count{0};
        std::vector<ShaderResource> resources{};
        std::vector<ShaderInput> inputs{};
    };

    enum class PipelineToggle : uint8_t { DISABLED, ENABLED };
    enum class PipelineCullMode : uint8_t { BACK, FRONT, NONE };
    enum class PipelineFrontFace : uint8_t { COUNTER_CLOCKWISE, CLOCKWISE };
    enum class PipelinePolygonMode : uint8_t { FILL, LINE, POINT };
    enum class PipelineCompareOp : uint8_t
    {
        LESS_OR_EQUAL, NEVER, LESS, EQUAL, GREATER, NOT_EQUAL, GREATER_OR_EQUAL, ALWAYS
    };
    enum class PipelineBlendFactor : uint8_t
    {
        ONE, ZERO, SOURCE_ALPHA, ONE_MINUS_SOURCE_ALPHA, DESTINATION_ALPHA,
        ONE_MINUS_DESTINATION_ALPHA, DESTINATION_COLOR, ONE_MINUS_DESTINATION_COLOR
    };
    enum class PipelineBlendOp : uint8_t { ADD, SUBTRACT, REVERSE_SUBTRACT, MIN, MAX };
    enum class PipelineStencilOp : uint8_t
    {
        KEEP, ZERO, REPLACE, INCREMENT_AND_CLAMP, DECREMENT_AND_CLAMP, INVERT,
        INCREMENT_AND_WRAP, DECREMENT_AND_WRAP
    };
    enum class PipelineSampleCount : uint8_t { ONE, TWO, FOUR, EIGHT, SIXTEEN, THIRTY_TWO, SIXTY_FOUR };

    struct PipelineRasterState
    {
        PipelineCullMode cull_mode{PipelineCullMode::BACK};
        PipelineFrontFace front_face{PipelineFrontFace::COUNTER_CLOCKWISE};
        PipelinePolygonMode polygon_mode{PipelinePolygonMode::FILL};
        PipelineToggle depth_clamp{PipelineToggle::DISABLED};
        PipelineToggle rasterizer_discard{PipelineToggle::DISABLED};
        PipelineToggle depth_bias{PipelineToggle::ENABLED};
        float depth_bias_constant{0.0f};
        float depth_bias_clamp{0.0f};
        float depth_bias_slope{0.0f};
        float line_width{1.0f};
    };

    struct PipelineDepthStencilState
    {
        PipelineToggle depth_test{PipelineToggle::ENABLED};
        PipelineToggle depth_write{PipelineToggle::ENABLED};
        PipelineCompareOp depth_compare{PipelineCompareOp::LESS_OR_EQUAL};
        PipelineToggle depth_bounds_test{PipelineToggle::DISABLED};
        float min_depth_bounds{0.0f};
        float max_depth_bounds{1.0f};
        PipelineToggle stencil_test{PipelineToggle::DISABLED};
        PipelineCompareOp stencil_compare{PipelineCompareOp::ALWAYS};
        PipelineStencilOp stencil_front_fail{PipelineStencilOp::KEEP};
        PipelineStencilOp stencil_front_depth_fail{PipelineStencilOp::KEEP};
        PipelineStencilOp stencil_front_pass{PipelineStencilOp::KEEP};
        PipelineStencilOp stencil_back_fail{PipelineStencilOp::KEEP};
        PipelineStencilOp stencil_back_depth_fail{PipelineStencilOp::KEEP};
        PipelineStencilOp stencil_back_pass{PipelineStencilOp::KEEP};
        uint32_t stencil_reference{0};
        uint32_t stencil_compare_mask{0xffffffffu};
        uint32_t stencil_write_mask{0xffffffffu};
    };

    struct PipelineBlendState
    {
        PipelineToggle enabled{PipelineToggle::ENABLED};
        uint32_t color_write_mask{0xf};
        PipelineBlendFactor source_color{PipelineBlendFactor::SOURCE_ALPHA};
        PipelineBlendFactor destination_color{PipelineBlendFactor::ONE_MINUS_SOURCE_ALPHA};
        PipelineBlendOp color_operation{PipelineBlendOp::ADD};
        PipelineBlendFactor source_alpha{PipelineBlendFactor::ONE};
        PipelineBlendFactor destination_alpha{PipelineBlendFactor::ZERO};
        PipelineBlendOp alpha_operation{PipelineBlendOp::ADD};
    };

    struct PipelineMultisampleState
    {
        PipelineSampleCount sample_count{PipelineSampleCount::ONE};
        PipelineToggle sample_shading{PipelineToggle::DISABLED};
        float min_sample_shading{0.0f};
        PipelineToggle alpha_to_coverage{PipelineToggle::ENABLED};
        PipelineToggle alpha_to_one{PipelineToggle::DISABLED};
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
        DLL const PipelineRasterState& GetRasterState() const;
        DLL const PipelineDepthStencilState& GetDepthStencilState() const;
        DLL const PipelineBlendState& GetBlendState() const;
        DLL const PipelineMultisampleState& GetMultisampleState() const;
#if 0
        DLL const std::string& GetVertexShaderCode() const;
        DLL const std::string& GetFragmentShaderCode() const;
        DLL const std::vector<std::tuple<UniformType, std::string >> & GetUniformDescriptors() const;
        DLL const std::vector<std::string>& GetSamplerDescriptors() const;
        DLL std::string GetProperties () const;
        DLL std::string GetAttributes () const;
        DLL uint32_t GetAttributeBitmap() const;
#endif
        /** Get the shader source code for the given shader stage, resolved for a
         * renderer. Each stage may carry per-renderer variants keyed by a
         * comma-separated renderer set (empty key = default); a renderer-specific
         * entry overrides the default, and an empty entry disables the stage.
         * @param aType The shader stage to retrieve code for.
         * @param aRenderer Name of the active renderer (e.g. "Vulkan"). Empty
         *        resolves to the default variant only.
         * @return String view of the resolved shader source, empty when the
         *         stage is absent or disabled for this renderer.
         */
        DLL const std::string_view GetShaderCode ( ShaderType aType, std::string_view aRenderer = {} ) const;

        /** Get the number of compute shader stages for a renderer.
         * Compute stages are ordered; index 0 is dispatched first.
         * @param aRenderer Name of the active renderer. Empty resolves to the
         *        default variant only.
         * @return The number of compute shader stages.
         */
        DLL uint32_t GetComputeStageCount ( std::string_view aRenderer = {} ) const;

        /** Get the shader source code for a compute stage by index, resolved for
         * a renderer.
         * @param aIndex Zero-based index of the compute stage.
         * @param aRenderer Name of the active renderer. Empty resolves to the
         *        default variant only.
         * @return String view of the compute shader source code.
         */
        DLL const std::string_view GetComputeShaderCode ( uint32_t aIndex, std::string_view aRenderer = {} ) const;

        /** Resolve generated interface metadata for one shader stage.
         * @param aType Graphics/compute stage type.
         * @param aRenderer Renderer name used for selector resolution.
         * @param aComputeStageIndex Ordered compute stage index; ignored for
         *        graphics stages.
         * @return Matching interface, or nullptr when no metadata was packed.
         */
        DLL const ShaderInterface* GetShaderInterface ( ShaderType aType,
                std::string_view aRenderer = {}, uint32_t aComputeStageIndex = 0 ) const;

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
        /** Per-stage renderer variants: selector (comma-separated renderer set,
         *  empty = default) -> shader source (empty = disabled). Compute stages
         *  map a selector to an ordered list of sources. Populated from the
         *  pipeline message; resolved against the active renderer by the
         *  Get*ShaderCode accessors. */
        std::array<std::unordered_map<std::string, std::string>, ShaderType::COUNT> mShaderVariants {};
        std::unordered_map<std::string, std::vector<std::string>> mComputeVariants {};
        std::unordered_map<std::string, std::vector<ShaderInterface>> mShaderInterfaces {};
        uint32_t mTopologyClass{ TOPOLOGY_CLASS_TRIANGLE };
        PipelineRasterState mRasterState{};
        PipelineDepthStencilState mDepthStencilState{};
        PipelineBlendState mBlendState{};
        PipelineMultisampleState mMultisampleState{};
    };
}
#endif

/*
Copyright (C) 2016-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MESH_H
#define AEONGAMES_MESH_H
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include "aeongames/AABB.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/Resource.hpp"

namespace AeonGames
{
    class MeshMsg;
    /** @brief Represents a polygon mesh with vertex attributes and index data. */
    class Mesh final : public Resource
    {
    public:
        /** @brief Semantic meaning of a vertex attribute, identified by CRC32 of its name. */
        enum AttributeSemantic : uint32_t
        {
            POSITION                      = "VertexPosition"_crc32,   ///< Vertex position.
            NORMAL                        = "VertexNormal"_crc32,     ///< Vertex normal.
            TANGENT                       = "VertexTangent"_crc32,    ///< Vertex tangent.
            BITANGENT                     = "VertexBitangent"_crc32,  ///< Vertex bitangent.
            TEXCOORD                      = "VertexUV"_crc32,         ///< Texture coordinate.
            WEIGHT_INDEX                  = "VertexWeightIndices"_crc32, ///< Bone weight indices.
            WEIGHT_VALUE                  = "VertexWeights"_crc32,    ///< Bone weight values.
            COLOR                         = "VertexColor"_crc32,      ///< Vertex color.
        };

        /** @brief Data type of a vertex attribute component. */
        enum AttributeType : uint8_t
        {
            BYTE             =  0, ///< Signed 8-bit integer.
            UNSIGNED_BYTE    =  1, ///< Unsigned 8-bit integer.
            SHORT            =  2, ///< Signed 16-bit integer.
            UNSIGNED_SHORT   =  3, ///< Unsigned 16-bit integer.
            HALF_FLOAT       =  4, ///< 16-bit floating point.
            INT              =  5, ///< Signed 32-bit integer.
            UNSIGNED_INT     =  6, ///< Unsigned 32-bit integer.
            FLOAT            =  7, ///< 32-bit floating point.
            FIXED            =  8, ///< Fixed-point.
            DOUBLE           =  9, ///< 64-bit floating point.
        };

        /** @brief Flags that modify how a vertex attribute is interpreted. */
        enum AttributeFlag : uint8_t
        {
            NORMALIZED       =  0b00000001, ///< Values are normalized to [0,1] or [-1,1].
            INTEGER          =  0b00000010, ///< Values are passed as integers (no conversion).
        };

        /** @brief CRC32-based binding location identifiers for descriptor sets. */
        enum  BindingLocations : uint32_t
        {
            MATRICES          = "Matrices"_crc32,        ///< Matrices binding.
            MATERIAL          = "Material"_crc32,        ///< Material binding.
            LIGHTS            = "Lights"_crc32,          ///< Per-frame lights binding.
            SAMPLERS          = "Samplers"_crc32,        ///< Samplers binding.
            CLUSTER_AABBS     = "ClusterAABBs"_crc32,    ///< Clustered shading: per-cluster view-space AABB list (SSBO).
            LIGHT_INDEX_LIST  = "LightIndexList"_crc32,  ///< Clustered shading: flat light-index list per cluster (SSBO).
            LIGHT_GRID        = "LightGrid"_crc32,       ///< Clustered shading: per-cluster (offset, count) into LightIndexList (SSBO).
            LIGHT_INDEX_COUNTER = "LightIndexCounter"_crc32, ///< Clustered shading: global atomic allocator for the flat LightIndexList (SSBO, R1).
            CLUSTER_ACTIVE    = "ClusterActive"_crc32,   ///< Clustered shading: per-cluster active flag set by the depth pre-pass mark stage (SSBO, R2).
            CLUSTER_PARAMS    = "ClusterParams"_crc32,   ///< Clustered shading: tile/slice dimensions and depth-slicing constants (UBO).
            SKINNING_MATRICES = "SkinningMatrices"_crc32, ///< Compute skinning: per-joint pose*inverse-bind matrices (SSBO, R4).
            SOURCE_VERTICES   = "SourceVertices"_crc32,  ///< Compute skinning: rest-pose source vertex buffer (SSBO, R4).
            SKINNED_VERTICES  = "SkinnedVertices"_crc32, ///< Compute skinning: skinned output vertex buffer (SSBO, R4).
            INSTANCE_MATRICES = "InstanceMatrices"_crc32, ///< Instanced rendering: per-instance model matrices indexed by instance id (SSBO).
            SHADOW_PARAMS     = "ShadowParams"_crc32,    ///< Directional shadow mapping: light view-projection and filtering params (UBO).
            SHADOW_MAP        = "ShadowMap"_crc32,       ///< Directional shadow mapping: depth shadow map sampled with comparison (sampler2DShadow).
            SPOT_SHADOW_PARAMS = "SpotShadowParams"_crc32, ///< Spot shadow mapping: per-caster light view-projections, caster light indices and filtering params (UBO).
            SPOT_SHADOW_MAP   = "SpotShadowMap"_crc32,   ///< Spot shadow mapping: depth shadow map array sampled with comparison (sampler2DArrayShadow).
            POINT_SHADOW_PARAMS = "PointShadowParams"_crc32, ///< Point shadow mapping: per-caster six-face light view-projections, caster positions/radii and filtering params (UBO).
            POINT_SHADOW_MAP  = "PointShadowMap"_crc32,  ///< Point shadow mapping: six-faces-per-caster depth array sampled with comparison (sampler2DArrayShadow).
        };

        /** @brief Type alias for the number of components in a vertex attribute. */
        using AttributeSize       = uint8_t;
        /** @brief Type alias for vertex attribute flag bits. */
        using AttributeFlags      = uint8_t;
        /** @brief Tuple describing a single vertex attribute (semantic, component count, type, flags). */
        using AttributeTuple = std::tuple<AttributeSemantic, AttributeSize, AttributeType, AttributeFlags>;
        /** @brief Default constructor. */
        DLL Mesh();
        /** @brief Destructor. */
        DLL ~Mesh() final;
        /** @brief Load mesh data from a protobuf message.
         *  @param aMeshMsg The protobuf message to load from.
         */
        DLL void LoadFromPBMsg ( const MeshMsg& aMeshMsg );
        /** @brief Load mesh data from a raw memory buffer.
         *  @param aBuffer Pointer to the buffer.
         *  @param aBufferSize Size of the buffer in bytes.
         */
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        /** @brief Unload mesh data and release resources. */
        DLL void Unload() final;
        /** @brief Get the list of vertex attributes.
         *  @return Const reference to the attribute tuple vector.
         */
        DLL const std::vector<AttributeTuple>& GetAttributes() const;
        /** @brief Get the size in bytes of a single index.
         *  @return Index element size.
         */
        DLL uint32_t GetIndexSize() const;
        /** @brief Get the total number of indices.
         *  @return Index count.
         */
        DLL uint32_t GetIndexCount() const;
        /** @brief Get the total number of vertices.
         *  @return Vertex count.
         */
        DLL uint32_t GetVertexCount() const;
        /** @brief Get the raw vertex data buffer.
         *  @return Const reference to the vertex byte vector.
         */
        DLL const std::vector<uint8_t>& GetVertexBuffer() const;
        /** @brief Get the raw index data buffer.
         *  @return Const reference to the index byte vector.
         */
        DLL const std::vector<uint8_t>& GetIndexBuffer() const;
        /** @brief Get the axis-aligned bounding box of the mesh.
         *  @return Const reference to the AABB.
         */
        DLL const AABB& GetAABB() const;
        /** @brief Get the stride (bytes per vertex) for the vertex buffer.
         *  @return Stride in bytes.
         */
        DLL size_t GetStride() const;
    private:
        AABB mAABB{};
        std::vector<uint8_t> mVertexBuffer{};
        std::vector<uint8_t> mIndexBuffer{};
        std::vector<AttributeTuple> mAttributes{};
        uint32_t mVertexCount{};
        uint32_t mIndexSize{};
        uint32_t mIndexCount{};
    };
    /** @brief Compute the total byte size of a single vertex attribute.
     *  @param aAttributeTuple Tuple describing the attribute.
     *  @return Size in bytes.
     */
    DLL size_t GetAttributeTotalSize ( const Mesh::AttributeTuple& aAttributeTuple );
}
#endif

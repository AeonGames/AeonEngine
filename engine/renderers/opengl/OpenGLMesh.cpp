/*
Copyright (C) 2016-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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

#include <cassert>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <cstring>
#include "OpenGLFunctions.hpp"
#include "aeongames/Mesh.hpp"
#include "OpenGLMesh.hpp"
#include "OpenGLRenderer.hpp"

namespace AeonGames
{
    OpenGLMesh::OpenGLMesh ( const OpenGLRenderer&  aOpenGLRenderer, const Mesh& aMesh ) :
        mOpenGLRenderer{aOpenGLRenderer}, mMesh{&aMesh}
    {
        assert ( aMesh.GetVertexCount() != 0 );
        // A mesh carrying per-vertex weights may be re-posed by the compute
        // skinning pass, which reads its rest-pose vertices as an SSBO and writes
        // a separate output buffer the draw binds. Such meshes keep their own
        // buffers. Weightless (static) meshes never skin, so they join the
        // renderer's shared geometry pool -- the foundation for GPU-driven
        // indirect drawing.
        bool has_weights = false;
        for ( const auto& attribute : aMesh.GetAttributes() )
        {
            if ( std::get<0> ( attribute ) == Mesh::WEIGHT_INDEX || std::get<0> ( attribute ) == Mesh::WEIGHT_VALUE )
            {
                has_weights = true;
                break;
            }
        }
        mStride = static_cast<uint32_t> ( aMesh.GetStride() );
        mPooled = !has_weights;

        if ( mPooled )
        {
            // Normalise indices to uint32 for the single shared index pool.
            std::vector<uint32_t> indices{};
            const uint32_t index_count = aMesh.GetIndexCount();
            if ( index_count )
            {
                indices.resize ( index_count );
                const uint8_t* source = aMesh.GetIndexBuffer().data();
                switch ( aMesh.GetIndexSize() )
                {
                case 1:
                    for ( uint32_t i = 0; i < index_count; ++i )
                    {
                        indices[i] = source[i];
                    }
                    break;
                case 2:
                    for ( uint32_t i = 0; i < index_count; ++i )
                    {
                        indices[i] = reinterpret_cast<const uint16_t*> ( source ) [i];
                    }
                    break;
                case 4:
                    std::memcpy ( indices.data(), source, static_cast<size_t> ( index_count ) * sizeof ( uint32_t ) );
                    break;
                default:
                    break;
                }
            }
            const OpenGLRenderer::GeometryAllocation allocation = mOpenGLRenderer.RegisterMeshGeometry (
                    mStride,
                    aMesh.GetVertexBuffer().data(),
                    static_cast<GLsizeiptr> ( aMesh.GetVertexBuffer().size() ),
                    index_count ? indices.data() : nullptr,
                    index_count );
            mBaseVertex = allocation.mBaseVertex;
            mFirstIndex = allocation.mFirstIndex;
            return;
        }

        // Skinned (weighted) mesh: private VBO + EBO, unchanged from before the pool.
        if ( aMesh.GetIndexCount() != 0 )
        {
            mIndexBuffer.Initialize
            ( static_cast<GLsizei> ( aMesh.GetIndexBuffer().size() ), GL_STATIC_DRAW, aMesh.GetIndexBuffer().data() );
        }
        mVertexBuffer.Initialize
        ( static_cast<GLsizei> ( aMesh.GetVertexBuffer().size() ), GL_STATIC_DRAW, aMesh.GetVertexBuffer().data() );
    }

    OpenGLMesh::~OpenGLMesh() = default;

    OpenGLMesh::OpenGLMesh ( OpenGLMesh&& aOpenGLMesh ) :
        mOpenGLRenderer{aOpenGLMesh.mOpenGLRenderer},
        mMesh{aOpenGLMesh.mMesh},
        mVertexBuffer{std::move ( aOpenGLMesh.mVertexBuffer ) },
        mIndexBuffer{std::move ( aOpenGLMesh.mIndexBuffer ) },
        mPooled{aOpenGLMesh.mPooled},
        mStride{aOpenGLMesh.mStride},
        mBaseVertex{aOpenGLMesh.mBaseVertex},
        mFirstIndex{aOpenGLMesh.mFirstIndex}
    {}

    static std::unordered_map<Mesh::AttributeType, GLsizei> MeshTypeToOGL
    {
        {Mesh::BYTE, GL_BYTE},
        {Mesh::UNSIGNED_BYTE, GL_UNSIGNED_BYTE},
        {Mesh::SHORT, GL_SHORT},
        {Mesh::UNSIGNED_SHORT, GL_UNSIGNED_SHORT},
        {Mesh::HALF_FLOAT, GL_HALF_FLOAT},
        {Mesh::INT, GL_INT},
        {Mesh::UNSIGNED_INT, GL_UNSIGNED_INT},
        {Mesh::FLOAT, GL_FLOAT},
        {Mesh::FIXED, GL_FIXED},
        {Mesh::DOUBLE, GL_DOUBLE},
    };

    void OpenGLMesh::Bind ( GLuint aSkinnedVertexBufferId ) const
    {
        const GLuint vertex_buffer_id =
            ( aSkinnedVertexBufferId != 0 ) ? aSkinnedVertexBufferId :
            mPooled ? mOpenGLRenderer.GetGeometryVertexBufferId ( mStride ) :
            mVertexBuffer.GetBufferId();
        glBindBuffer ( GL_ARRAY_BUFFER, vertex_buffer_id );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLMesh::EnableAttributes ( const std::vector<OpenGLVariable>& aAttributes, size_t aBaseOffset, size_t aStrideOverride ) const
    {
        for ( GLuint i = 0; i < 8; ++i )
        {
            glDisableVertexAttribArray ( i );
        }

        const GLsizei stride = static_cast<GLsizei> ( ( aStrideOverride != 0 ) ? aStrideOverride : mMesh->GetStride() );
        size_t offset{0};
        for ( auto& attribute : mMesh->GetAttributes() )
        {
            auto it = std::lower_bound ( aAttributes.begin(), aAttributes.end(), std::get<0> ( attribute ),
                                         [] ( const OpenGLVariable & a, uint32_t b )
            {
                return a.name < b;
            } );
            if ( it == aAttributes.end() || it->name != std::get<0> ( attribute ) )
            {
                /* If we get here, it means the attribute is not present in aAttributes,
                    but it is present in the mesh, so skip it and continue. */
                offset += GetAttributeTotalSize ( attribute );
                continue;
            }

            glEnableVertexAttribArray ( it->location );
            OPENGL_CHECK_ERROR_THROW;
            if ( ! ( std::get<3> ( attribute ) & Mesh::AttributeFlag::INTEGER ) )
            {
                glVertexAttribPointer (
                    it->location,
                    std::get<1> ( attribute ),
                    MeshTypeToOGL[std::get<2> ( attribute )],
                    std::get<3> ( attribute ) & Mesh::AttributeFlag::NORMALIZED,
                    stride,
                    reinterpret_cast<const void*> ( offset + aBaseOffset ) );
                OPENGL_CHECK_ERROR_THROW;
            }
            else
            {
                glVertexAttribIPointer (
                    it->location,
                    std::get<1> ( attribute ),
                    MeshTypeToOGL[std::get<2> ( attribute )],
                    stride,
                    reinterpret_cast<const void*> ( offset + aBaseOffset ) );
            }
            offset += GetAttributeTotalSize ( attribute );
        }
        //---Index Buffer---
        if ( mPooled )
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mOpenGLRenderer.GetGeometryIndexBufferId() );
        }
        else if ( mIndexBuffer.GetSize() != 0 )
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer.GetBufferId() );
        }
        else
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLMesh::DisableAttributes ( const std::vector<OpenGLVariable>& aAttributes ) const
    {
        for ( const auto& attribute : aAttributes )
        {
            glDisableVertexAttribArray ( attribute.location );
        }
    }

    GLuint OpenGLMesh::GetVertexBufferId() const
    {
        return mVertexBuffer.GetBufferId();
    }
}

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
#include "OpenGLFunctions.hpp"
#include "aeongames/Mesh.hpp"
#include "OpenGLMesh.hpp"

namespace AeonGames
{
    OpenGLMesh::OpenGLMesh ( const OpenGLRenderer&  aOpenGLRenderer, const Mesh& aMesh ) :
        mOpenGLRenderer{aOpenGLRenderer}, mMesh{&aMesh}
    {
        if ( aMesh.GetIndexCount() != 0 )
        {
            mIndexBuffer.Initialize
            ( static_cast<GLsizei> ( aMesh.GetIndexBuffer().size() ), GL_STATIC_DRAW, aMesh.GetIndexBuffer().data() );
        }
        assert ( aMesh.GetVertexCount() != 0 );
        mVertexBuffer.Initialize
        ( static_cast<GLsizei> ( aMesh.GetVertexBuffer().size() ), GL_STATIC_DRAW, aMesh.GetVertexBuffer().data() );
    }

    OpenGLMesh::~OpenGLMesh() = default;

    OpenGLMesh::OpenGLMesh ( OpenGLMesh&& aOpenGLMesh ) :
        mOpenGLRenderer{aOpenGLMesh.mOpenGLRenderer},
        mMesh{aOpenGLMesh.mMesh},
        mVertexBuffer{std::move ( aOpenGLMesh.mVertexBuffer ) },
        mIndexBuffer{std::move ( aOpenGLMesh.mIndexBuffer ) }
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

    void OpenGLMesh::Bind() const
    {
        glBindBuffer ( GL_ARRAY_BUFFER, mVertexBuffer.GetBufferId() );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLMesh::EnableAttributes ( const std::vector<OpenGLVariable>& aAttributes ) const
    {
        for ( GLuint i = 0; i < 8; ++i )
        {
            glDisableVertexAttribArray ( i );
        }

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
                    static_cast<GLsizei> ( mMesh->GetStride() ),
                    reinterpret_cast<const void*> ( offset ) );
                OPENGL_CHECK_ERROR_THROW;
            }
            else
            {
                glVertexAttribIPointer (
                    it->location,
                    std::get<1> ( attribute ),
                    MeshTypeToOGL[std::get<2> ( attribute )],
                    static_cast<GLsizei> ( mMesh->GetStride() ),
                    reinterpret_cast<const void*> ( offset ) );
            }
            offset += GetAttributeTotalSize ( attribute );
        }
        //---Index Buffer---
        if ( mIndexBuffer.GetSize() != 0 )
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
}

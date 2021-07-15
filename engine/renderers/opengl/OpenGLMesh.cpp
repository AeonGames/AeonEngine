/*
Copyright (C) 2016-2019,2021 Rodrigo Jose Hernandez Cordoba

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

#include <fstream>
#include <sstream>
#include <exception>
#include <utility>
#include <vector>
#include <cassert>
#include <cstring>
#include "aeongames/Utilities.h"
#include "OpenGLFunctions.h"
#include "aeongames/Mesh.h"
#include "OpenGLMesh.h"

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

    void OpenGLMesh::Bind() const
    {
        glBindBuffer ( GL_ARRAY_BUFFER, mVertexBuffer.GetBufferId() );
        OPENGL_CHECK_ERROR_THROW;

        size_t offset{0};
        if ( mMesh->GetVertexFlags() & Mesh::POSITION_BIT )
        {
            glEnableVertexAttribArray ( 0 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 0 );
        }

        if ( mMesh->GetVertexFlags() & Mesh::NORMAL_BIT )
        {
            glEnableVertexAttribArray ( 1 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 1 );
        }

        if ( mMesh->GetVertexFlags() & Mesh::TANGENT_BIT )
        {
            glEnableVertexAttribArray ( 2 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 2 );
        }

        if ( mMesh->GetVertexFlags() & Mesh::BITANGENT_BIT )
        {
            glEnableVertexAttribArray ( 3 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 3, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 3 );
        }

        if ( mMesh->GetVertexFlags() & Mesh::UV_BIT )
        {
            glEnableVertexAttribArray ( 4 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 4, 2, GL_FLOAT, GL_FALSE, mMesh->GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 2;
        }
        else
        {
            glDisableVertexAttribArray ( 4 );
        }

        if ( mMesh->GetVertexFlags() & Mesh::WEIGHT_IDX_BIT )
        {
            glEnableVertexAttribArray ( 5 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribIPointer ( 5, 4, GL_UNSIGNED_BYTE, mMesh->GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
        }
        else
        {
            glDisableVertexAttribArray ( 5 );
        }

        if ( mMesh->GetVertexFlags() & Mesh::WEIGHT_BIT )
        {
            glEnableVertexAttribArray ( 6 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 6, 4, GL_UNSIGNED_BYTE, GL_TRUE, mMesh->GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
        }
        else
        {
            glDisableVertexAttribArray ( 6 );
        }

        if ( mMesh->GetVertexFlags() & Mesh::COLOR_BIT )
        {
            glEnableVertexAttribArray ( 7 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 7, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 7 );
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
}

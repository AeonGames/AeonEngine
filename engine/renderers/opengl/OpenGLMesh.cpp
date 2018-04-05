/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/Utilities.h"
#include "OpenGLFunctions.h"
#include "aeongames/Mesh.h"
#include "OpenGLMesh.h"

namespace AeonGames
{
    OpenGLMesh::OpenGLMesh ( const std::shared_ptr<const Mesh>&  aMesh, const std::shared_ptr<const OpenGLRenderer>& aOpenGLRenderer ) :
        mMesh ( aMesh )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }
    OpenGLMesh::~OpenGLMesh()
    {
        Finalize();
    }

    void OpenGLMesh::Render() const
    {
        glBindVertexArray ( mArray );
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( mMesh->GetIndexCount() )
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            glDrawElements ( GL_TRIANGLES, mMesh->GetIndexCount(),
                             0x1400 | mMesh->GetIndexType(), nullptr );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glDrawArrays ( GL_TRIANGLES, 0, mMesh->GetVertexCount() );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLMesh::Initialize()
    {
        glGenVertexArrays ( 1, &mArray );
        OPENGL_CHECK_ERROR_THROW;
        glBindVertexArray ( mArray );
        OPENGL_CHECK_ERROR_THROW;
        glGenBuffers ( 1, &mVertexBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBindBuffer ( GL_ARRAY_BUFFER, mVertexBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBufferData ( GL_ARRAY_BUFFER, mMesh->GetVertexBuffer().size(), mMesh->GetVertexBuffer().data(), GL_STATIC_DRAW );
        OPENGL_CHECK_ERROR_THROW;

        uint8_t* offset = nullptr;
        if ( mMesh->GetVertexFlags() & Mesh::POSITION_BIT )
        {
            glEnableVertexAttribArray ( 0 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride(), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mMesh->GetVertexFlags() & Mesh::NORMAL_BIT )
        {
            glEnableVertexAttribArray ( 1 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride(), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mMesh->GetVertexFlags() & Mesh::TANGENT_BIT )
        {
            glEnableVertexAttribArray ( 2 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride(), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mMesh->GetVertexFlags() & Mesh::BITANGENT_BIT )
        {
            glEnableVertexAttribArray ( 3 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 3, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride(), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mMesh->GetVertexFlags() & Mesh::UV_BIT )
        {
            glEnableVertexAttribArray ( 4 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 4, 2, GL_FLOAT, GL_FALSE, mMesh->GetStride(), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 2;
        }

        if ( mMesh->GetVertexFlags() & Mesh::WEIGHT_BIT )
        {
            glEnableVertexAttribArray ( 5 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribIPointer ( 5, 4, GL_UNSIGNED_BYTE, mMesh->GetStride(), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
            glEnableVertexAttribArray ( 6 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 6, 4, GL_UNSIGNED_BYTE, GL_TRUE, mMesh->GetStride(), offset );
            OPENGL_CHECK_ERROR_THROW;
            //offset += sizeof ( uint8_t ) * 4;
        }
        //---Index Buffer---
        if ( mMesh->GetIndexCount() )
        {
            glGenBuffers ( 1, &mIndexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBufferData ( GL_ELEMENT_ARRAY_BUFFER, mMesh->GetIndexBuffer().size(), mMesh->GetIndexBuffer().data(), GL_STATIC_DRAW );
            OPENGL_CHECK_ERROR_THROW;
        }

    }

    void OpenGLMesh::Finalize()
    {
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( glIsVertexArray ( mArray ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteVertexArrays ( 1, &mArray );
            OPENGL_CHECK_ERROR_NO_THROW;
            mArray = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( glIsBuffer ( mVertexBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteBuffers ( 1, &mVertexBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mVertexBuffer = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( glIsBuffer ( mIndexBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteBuffers ( 1, &mIndexBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mIndexBuffer = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
    }
}

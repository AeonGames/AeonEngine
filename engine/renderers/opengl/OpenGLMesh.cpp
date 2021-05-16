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
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/Utilities.h"
#include "OpenGLFunctions.h"
#include "aeongames/CRC.h"
#include "OpenGLMesh.h"

namespace AeonGames
{
    OpenGLMesh::OpenGLMesh ( uint32_t aPath )
    {
        if ( aPath )
        {
            Resource::Load ( aPath );
        }
    }

    OpenGLMesh::~OpenGLMesh()
    {
        Unload();
    }

    void OpenGLMesh::BindVertexArray() const
    {
        BindBuffers();
    }

    GLuint OpenGLMesh::GetVertexBufferId() const
    {
        return mVertexBuffer;
    }

    GLuint OpenGLMesh::GetIndexBufferId() const
    {
        return mIndexBuffer;
    }

    GLenum OpenGLMesh::GetIndexType() const
    {
        switch ( GetIndexSize() )
        {
        case 1:
            return GL_UNSIGNED_BYTE;
        case 2:
            return GL_UNSIGNED_SHORT;
        case 4:
            return GL_UNSIGNED_INT;
        };
        throw std::runtime_error ( "Invalid Index Size." );
    }

    void OpenGLMesh::BindBuffers() const
    {
        glBindBuffer ( GL_ARRAY_BUFFER, mVertexBuffer );
        OPENGL_CHECK_ERROR_THROW;

        size_t offset{0};
        if ( GetVertexFlags() & Mesh::POSITION_BIT )
        {
            glEnableVertexAttribArray ( 0 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, GetStride ( GetVertexFlags() ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 0 );
        }

        if ( GetVertexFlags() & Mesh::NORMAL_BIT )
        {
            glEnableVertexAttribArray ( 1 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, GetStride ( GetVertexFlags() ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 1 );
        }

        if ( GetVertexFlags() & Mesh::TANGENT_BIT )
        {
            glEnableVertexAttribArray ( 2 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE, GetStride ( GetVertexFlags() ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 2 );
        }

        if ( GetVertexFlags() & Mesh::BITANGENT_BIT )
        {
            glEnableVertexAttribArray ( 3 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 3, 3, GL_FLOAT, GL_FALSE, GetStride ( GetVertexFlags() ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 3 );
        }

        if ( GetVertexFlags() & Mesh::UV_BIT )
        {
            glEnableVertexAttribArray ( 4 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 4, 2, GL_FLOAT, GL_FALSE, GetStride ( GetVertexFlags() ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 2;
        }
        else
        {
            glDisableVertexAttribArray ( 4 );
        }

        if ( GetVertexFlags() & Mesh::WEIGHT_IDX_BIT )
        {
            glEnableVertexAttribArray ( 5 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribIPointer ( 5, 4, GL_UNSIGNED_BYTE, GetStride ( GetVertexFlags() ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
        }
        else
        {
            glDisableVertexAttribArray ( 5 );
        }

        if ( GetVertexFlags() & Mesh::WEIGHT_BIT )
        {
            glEnableVertexAttribArray ( 6 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 6, 4, GL_UNSIGNED_BYTE, GL_TRUE, GetStride ( GetVertexFlags() ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
        }
        else
        {
            glDisableVertexAttribArray ( 6 );
        }

        if ( GetVertexFlags() & Mesh::COLOR_BIT )
        {
            glEnableVertexAttribArray ( 7 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 7, 3, GL_FLOAT, GL_FALSE, GetStride ( GetVertexFlags() ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 7 );
        }

        //---Index Buffer---
        if ( GetIndexCount() )
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer );
        }
        else
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLMesh::Unload ()
    {
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

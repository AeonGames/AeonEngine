/*
Copyright (C) 2016-2019 Rodrigo Jose Hernandez Cordoba

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
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/AeonEngine.h"
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
            Load ( aPath );
        }
    }

    OpenGLMesh::~OpenGLMesh()
    {
        Unload();
    }

    void OpenGLMesh::BindVertexArray() const
    {
#ifdef SINGLE_VAO
        BindBuffers();
#else
        glBindVertexArray ( mArray );
#endif
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
        switch ( mIndexSize )
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

    void OpenGLMesh::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }
    void OpenGLMesh::Load ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        try
        {
            Load ( buffer.data(), buffer.size() );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }
    void OpenGLMesh::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static MeshBuffer mesh_buffer;
        LoadProtoBufObject ( mesh_buffer, aBuffer, aBufferSize, "AEONMSH" );
        Load ( mesh_buffer );
        mesh_buffer.Clear();
    }
    void OpenGLMesh::Load ( const MeshBuffer& aMeshBuffer )
    {
        mAABB = AABB
        {
            {
                aMeshBuffer.center().x(),
                aMeshBuffer.center().y(),
                aMeshBuffer.center().z()
            },
            {
                aMeshBuffer.radii().x(),
                aMeshBuffer.radii().y(),
                aMeshBuffer.radii().z()
            }
        };

        mVertexCount = aMeshBuffer.vertexcount();
        mIndexCount = aMeshBuffer.indexcount();
        mIndexSize = aMeshBuffer.indexsize();
        mVertexFlags = aMeshBuffer.vertexflags();

        // OpenGL Specific Code:
        ///@todo Use OpenGLBuffer class instead of raw GL ids

#ifndef SINGLE_VAO
        glGenVertexArrays ( 1, &mArray );
        OPENGL_CHECK_ERROR_THROW;
        glBindVertexArray ( mArray );
        OPENGL_CHECK_ERROR_THROW;
#endif
        glGenBuffers ( 1, &mVertexBuffer );
        OPENGL_CHECK_ERROR_THROW;

        glBindBuffer ( GL_ARRAY_BUFFER, mVertexBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBufferData ( GL_ARRAY_BUFFER, aMeshBuffer.vertexbuffer().size(), aMeshBuffer.vertexbuffer().data(), GL_STATIC_DRAW );
        OPENGL_CHECK_ERROR_THROW;

        //---Index Buffer---
        if ( mIndexCount )
        {
            glGenBuffers ( 1, &mIndexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBufferData ( GL_ELEMENT_ARRAY_BUFFER, aMeshBuffer.indexbuffer().size(), aMeshBuffer.indexbuffer().data(), GL_STATIC_DRAW );
            OPENGL_CHECK_ERROR_THROW;
        }
#ifndef SINGLE_VAO
        BindBuffers();
#endif
    }

    void OpenGLMesh::BindBuffers() const
    {
        glBindBuffer ( GL_ARRAY_BUFFER, mVertexBuffer );
        OPENGL_CHECK_ERROR_THROW;

        size_t offset{0};
        if ( mVertexFlags & Mesh::POSITION_BIT )
        {
            glEnableVertexAttribArray ( 0 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, GetStride ( mVertexFlags ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mVertexFlags & Mesh::NORMAL_BIT )
        {
            glEnableVertexAttribArray ( 1 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, GetStride ( mVertexFlags ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mVertexFlags & Mesh::TANGENT_BIT )
        {
            glEnableVertexAttribArray ( 2 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE, GetStride ( mVertexFlags ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mVertexFlags & Mesh::BITANGENT_BIT )
        {
            glEnableVertexAttribArray ( 3 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 3, 3, GL_FLOAT, GL_FALSE, GetStride ( mVertexFlags ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mVertexFlags & Mesh::UV_BIT )
        {
            glEnableVertexAttribArray ( 4 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 4, 2, GL_FLOAT, GL_FALSE, GetStride ( mVertexFlags ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 2;
        }

        if ( mVertexFlags & Mesh::WEIGHT_IDX_BIT )
        {
            glEnableVertexAttribArray ( 5 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribIPointer ( 5, 4, GL_UNSIGNED_BYTE, GetStride ( mVertexFlags ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
        }

        if ( mVertexFlags & Mesh::WEIGHT_BIT )
        {
            glEnableVertexAttribArray ( 6 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 6, 4, GL_UNSIGNED_BYTE, GL_TRUE, GetStride ( mVertexFlags ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
        }

        if ( mVertexFlags & Mesh::COLOR_BIT )
        {
            glEnableVertexAttribArray ( 7 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 7, 3, GL_FLOAT, GL_FALSE, GetStride ( mVertexFlags ), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        //---Index Buffer---
        if ( mIndexCount )
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer );
            OPENGL_CHECK_ERROR_THROW;
        }
    }

    void OpenGLMesh::Unload ()
    {
#ifndef SINGLE_VAO
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( glIsVertexArray ( mArray ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteVertexArrays ( 1, &mArray );
            OPENGL_CHECK_ERROR_NO_THROW;
            mArray = 0;
        }
#endif
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

    uint32_t OpenGLMesh::GetIndexSize () const
    {
        return mIndexSize;
    }

    uint32_t OpenGLMesh::GetIndexCount() const
    {
        return mIndexCount;
    }

    uint32_t OpenGLMesh::GetVertexCount() const
    {
        return mVertexCount;
    }

    const AABB& OpenGLMesh::GetAABB() const
    {
        return mAABB;
    }
}

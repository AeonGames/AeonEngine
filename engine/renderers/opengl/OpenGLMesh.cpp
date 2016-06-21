/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#include <vector>
#include <cassert>
#include <cstring>

#ifdef __unix__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/text_format.h>
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "OpenGLFunctions.h"
#include "OpenGLMesh.h"

namespace AeonGames
{
    OpenGLMesh::OpenGLMesh ( const std::string& aFilename )
try :
        mFilename ( aFilename ), mArray ( 0 ), mBuffer ( 0 )
    {
        Initialize();
    }
    catch ( ... )
    {
        Finalize();
        throw;
    }

    OpenGLMesh::~OpenGLMesh()
    {
        Finalize();
    }

    void OpenGLMesh::Render() const
    {
        glBindVertexArray ( mArray );
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( mIndexCount )
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer );
            glDrawElements ( GL_TRIANGLES, mIndexCount, mIndexType, 0 );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glDrawArrays ( GL_TRIANGLES, 0, mVertexCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLMesh::Initialize()
    {
        struct stat stat_buffer;
        if ( stat ( mFilename.c_str(), &stat_buffer ) != 0 )
        {
            std::ostringstream stream;
            stream << "File " << mFilename << " Not Found (error code:" << errno << ")";
            throw std::runtime_error ( stream.str().c_str() );
        }
        std::ifstream file;
        file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
        file.open ( mFilename, std::ifstream::in | std::ifstream::binary );
        char magick_number[8] = { 0 };
        file.read ( magick_number, sizeof ( magick_number ) );
        file.exceptions ( std::ifstream::badbit );
        static MeshBuffer mesh_buffer;
        if ( strncmp ( magick_number, "AEONMSH", 7 ) )
        {
            file.close();
            std::ostringstream stream;
            stream << "File" << mFilename << " Is not in AeonGames MSH format.";
            throw std::runtime_error ( stream.str().c_str() );
        }
        else if ( magick_number[7] == '\0' )
        {
            if ( !mesh_buffer.ParseFromIstream ( &file ) )
            {
                throw std::runtime_error ( "Mesh Parse failed." );
            }
        }
        else
        {
            std::string text ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
            if ( !google::protobuf::TextFormat::ParseFromString ( text, &mesh_buffer ) )
            {
                throw std::runtime_error ( "Text mesh parsing failed." );
            }
        }
        file.close();

        mVertexCount = mesh_buffer.vertexcount();
        mIndexCount = mesh_buffer.indexcount();
        mIndexType = 0x1400 | mesh_buffer.indextype();
        mIndexOffset = GetStride ( mesh_buffer.vertexflags() ) * mVertexCount;

        glGenVertexArrays ( 1, &mArray );
        OPENGL_CHECK_ERROR_THROW;
        glBindVertexArray ( mArray );
        OPENGL_CHECK_ERROR_THROW;
        glGenBuffers ( 1, &mBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBindBuffer ( GL_ARRAY_BUFFER, mBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBufferData ( GL_ARRAY_BUFFER, mesh_buffer.vertexbuffer().length(), mesh_buffer.vertexbuffer().data(), GL_STATIC_DRAW );
        OPENGL_CHECK_ERROR_THROW;

        uint8_t* offset = nullptr;
        if ( mesh_buffer.vertexflags() & POSITION_MASK )
        {
            glEnableVertexAttribArray ( 0 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, GetStride ( mesh_buffer.vertexflags() ), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mesh_buffer.vertexflags() & NORMAL_MASK )
        {
            glEnableVertexAttribArray ( 1 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, GetStride ( mesh_buffer.vertexflags() ), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }

        if ( mesh_buffer.vertexflags() & UV_MASK )
        {
            glEnableVertexAttribArray ( 2 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 2, 2, GL_FLOAT, GL_FALSE, GetStride ( mesh_buffer.vertexflags() ), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 2;
        }

        if ( mesh_buffer.vertexflags() & WEIGHT_MASK )
        {
            glEnableVertexAttribArray ( 3 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 3, 4, GL_UNSIGNED_BYTE, GL_FALSE, GetStride ( mesh_buffer.vertexflags() ), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
            glEnableVertexAttribArray ( 4 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, GetStride ( mesh_buffer.vertexflags() ), offset );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
        }
        //---Index Buffer---
        if ( mIndexCount )
        {
            glGenBuffers ( 1, &mIndexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBufferData ( GL_ELEMENT_ARRAY_BUFFER,
                           mesh_buffer.vertexbuffer().length() - mIndexOffset,
                           mesh_buffer.vertexbuffer().data() + mIndexOffset, GL_STATIC_DRAW );
            OPENGL_CHECK_ERROR_THROW;
        }
        mesh_buffer.Clear();
    }

    void OpenGLMesh::Finalize()
    {
        if ( glIsVertexArray ( mArray ) )
        {
            glDeleteVertexArrays ( 1, &mArray );
            mArray = 0;
        }
        if ( glIsBuffer ( mBuffer ) )
        {
            glDeleteBuffers ( 1, &mBuffer );
            mBuffer = 0;
        }
        if ( glIsBuffer ( mIndexBuffer ) )
        {
            glDeleteBuffers ( 1, &mIndexBuffer );
            mIndexBuffer = 0;
        }
    }

    uint32_t OpenGLMesh::GetStride ( uint32_t aFlags ) const
    {
        uint32_t stride = 0;
        if ( aFlags & POSITION_MASK )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & NORMAL_MASK )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & UV_MASK )
        {
            stride += sizeof ( float ) * 2;
        }
        if ( aFlags & WEIGHT_MASK )
        {
            stride += sizeof ( uint8_t ) * 8;
        }
        return stride;
    }
    uint32_t OpenGLMesh::GetIndexSize ( uint32_t aIndexType ) const
    {
        switch ( 0x1400 | aIndexType )
        {
        case GL_UNSIGNED_BYTE:
            return 1;
        case  GL_UNSIGNED_SHORT:
            return 2;
        case GL_UNSIGNED_INT:
            return 4;
        };
        assert ( 0 && "Invalid Index Type." );
        return 0;
    }
}

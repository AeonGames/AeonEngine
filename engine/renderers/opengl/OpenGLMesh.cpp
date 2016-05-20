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

#include "OpenGLFunctions.h"
#include "OpenGLMesh.h"
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

namespace AeonGames
{
    OpenGLMesh::OpenGLMesh ( const std::string& aFilename )
try :
        mFilename ( aFilename ), mHeader{}, mArray ( 0 ), mBuffer ( 0 )
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
        file.read ( reinterpret_cast<char*> ( &mHeader ), sizeof ( MSHHeader ) );
        if ( strncmp ( mHeader.id, "AEONMSH\0", 8 ) )
        {
            file.close();
            std::ostringstream stream;
            stream << "File" << mFilename << " Is not in AeonGames MSH format.";
            throw std::runtime_error ( stream.str().c_str() );
        }

        size_t buffer_size = ( GetStride ( mHeader.vertex_flags ) * mHeader.vertex_count ) + ( GetIndexSize ( mHeader.index_type ) * mHeader.index_count );
        std::vector<char> buffer ( buffer_size );
        file.read ( buffer.data(), buffer_size );
        file.close();

        glGenVertexArrays ( 1, &mArray );
        glBindVertexArray ( mArray );
        glGenBuffers ( 1, &mBuffer );
        glBindBuffer ( GL_ARRAY_BUFFER, mBuffer );
        glBufferData ( GL_ARRAY_BUFFER, buffer_size, buffer.data(), GL_STATIC_DRAW );

        uint8_t* offset = nullptr;
        if ( mHeader.vertex_flags & POSITION_MASK )
        {
            glEnableVertexAttribArray ( 0 );
            glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, GetStride ( mHeader.vertex_flags ), offset );
            offset += sizeof ( float ) * 3;
        }

        if ( mHeader.vertex_flags & NORMAL_MASK )
        {
            glEnableVertexAttribArray ( 1 );
            glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, GetStride ( mHeader.vertex_flags ), offset );
            offset += sizeof ( float ) * 3;
        }

        if ( mHeader.vertex_flags & UV_MASK )
        {
            glEnableVertexAttribArray ( 2 );
            glVertexAttribPointer ( 2, 2, GL_FLOAT, GL_FALSE, GetStride ( mHeader.vertex_flags ), offset );
            offset += sizeof ( float ) * 2;
        }

        if ( mHeader.vertex_flags & WEIGHT_MASK )
        {
            glEnableVertexAttribArray ( 3 );
            glVertexAttribPointer ( 3, 4, GL_UNSIGNED_BYTE, GL_FALSE, GetStride ( mHeader.vertex_flags ), offset );
            offset += sizeof ( uint8_t ) * 4;
            glEnableVertexAttribArray ( 4 );
            glVertexAttribPointer ( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, GetStride ( mHeader.vertex_flags ), offset );
            offset += sizeof ( uint8_t ) * 4;
        }
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

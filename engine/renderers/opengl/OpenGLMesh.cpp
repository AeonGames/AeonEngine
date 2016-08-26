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

#ifdef __GNUG__
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
        :
        mFilename ( aFilename )
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
        for ( auto& i : mTriangleGroups )
        {
            glBindVertexArray ( i.mArray );
            OPENGL_CHECK_ERROR_NO_THROW;
            if ( i.mIndexCount )
            {
                glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, i.mIndexBuffer );
                OPENGL_CHECK_ERROR_NO_THROW;
                glDrawElements ( GL_TRIANGLES, i.mIndexCount, i.mIndexType, 0 );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            else
            {
                glDrawArrays ( GL_TRIANGLES, 0, i.mVertexCount );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
        }
    }

    const float * const OpenGLMesh::GetCenterRadii() const
    {
        return mCenterRadii;
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

        if ( mesh_buffer.trianglegroup().size() == 1 )
        {
            /* This avoids having to duplicate the values on single triangle group meshes.*/
            // Extract Center
            mCenterRadii[0] = mesh_buffer.trianglegroup().Get ( 0 ).center().x();
            mCenterRadii[1] = mesh_buffer.trianglegroup().Get ( 0 ).center().y();
            mCenterRadii[2] = mesh_buffer.trianglegroup().Get ( 0 ).center().z();
            // Extract Radius
            mCenterRadii[3] = mesh_buffer.trianglegroup().Get ( 0 ).radii().x();
            mCenterRadii[4] = mesh_buffer.trianglegroup().Get ( 0 ).radii().y();
            mCenterRadii[5] = mesh_buffer.trianglegroup().Get ( 0 ).radii().z();
        }
        else
        {
            // Extract Center
            mCenterRadii[0] = mesh_buffer.center().x();
            mCenterRadii[1] = mesh_buffer.center().y();
            mCenterRadii[2] = mesh_buffer.center().z();
            // Extract Radius
            mCenterRadii[3] = mesh_buffer.radii().x();
            mCenterRadii[4] = mesh_buffer.radii().y();
            mCenterRadii[5] = mesh_buffer.radii().z();
        }

        mTriangleGroups.reserve ( mesh_buffer.trianglegroup().size() );
        for ( auto& i : mesh_buffer.trianglegroup() )
        {
            mTriangleGroups.emplace_back();

            // Extract Center
            mTriangleGroups.back().mCenterRadii[0] = i.center().x();
            mTriangleGroups.back().mCenterRadii[1] = i.center().y();
            mTriangleGroups.back().mCenterRadii[2] = i.center().z();
            // Extract Radius
            mTriangleGroups.back().mCenterRadii[3] = i.radii().x();
            mTriangleGroups.back().mCenterRadii[4] = i.radii().y();
            mTriangleGroups.back().mCenterRadii[5] = i.radii().z();

            mTriangleGroups.back().mVertexCount = i.vertexcount();
            mTriangleGroups.back().mIndexCount = i.indexcount();
            mTriangleGroups.back().mIndexType = 0x1400 | i.indextype();

            glGenVertexArrays ( 1, &mTriangleGroups.back().mArray );
            OPENGL_CHECK_ERROR_THROW;
            glBindVertexArray ( mTriangleGroups.back().mArray );
            OPENGL_CHECK_ERROR_THROW;
            glGenBuffers ( 1, &mTriangleGroups.back().mVertexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBindBuffer ( GL_ARRAY_BUFFER, mTriangleGroups.back().mVertexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBufferData ( GL_ARRAY_BUFFER, i.vertexbuffer().size(), i.vertexbuffer().data(), GL_STATIC_DRAW );
            OPENGL_CHECK_ERROR_THROW;

            uint8_t* offset = nullptr;
            if ( i.vertexflags() & POSITION_BIT )
            {
                glEnableVertexAttribArray ( 0 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, GetStride ( i.vertexflags() ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 3;
            }

            if ( i.vertexflags() & NORMAL_BIT )
            {
                glEnableVertexAttribArray ( 1 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, GetStride ( i.vertexflags() ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 3;
            }

            if ( i.vertexflags() & TANGENT_BIT )
            {
                glEnableVertexAttribArray ( 2 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE, GetStride ( i.vertexflags() ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 3;
            }

            if ( i.vertexflags() & BITANGENT_BIT )
            {
                glEnableVertexAttribArray ( 3 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 3, 3, GL_FLOAT, GL_FALSE, GetStride ( i.vertexflags() ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 3;
            }

            if ( i.vertexflags() & UV_BIT )
            {
                glEnableVertexAttribArray ( 4 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 4, 2, GL_FLOAT, GL_FALSE, GetStride ( i.vertexflags() ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 2;
            }

            if ( i.vertexflags() & WEIGHT_BIT )
            {
                glEnableVertexAttribArray ( 3 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 3, 4, GL_UNSIGNED_BYTE, GL_FALSE, GetStride ( i.vertexflags() ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( uint8_t ) * 4;
                glEnableVertexAttribArray ( 4 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, GetStride ( i.vertexflags() ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( uint8_t ) * 4;
            }
            //---Index Buffer---
            if ( mTriangleGroups.back().mIndexCount )
            {
                glGenBuffers ( 1, &mTriangleGroups.back().mIndexBuffer );
                OPENGL_CHECK_ERROR_THROW;
                glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mTriangleGroups.back().mIndexBuffer );
                OPENGL_CHECK_ERROR_THROW;
                glBufferData ( GL_ARRAY_BUFFER, i.vertexbuffer().size(), i.vertexbuffer().data(), GL_STATIC_DRAW );

                glBufferData ( GL_ELEMENT_ARRAY_BUFFER, i.indexbuffer().size(), i.indexbuffer().data(), GL_STATIC_DRAW );
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        mesh_buffer.Clear();
    }

    void OpenGLMesh::Finalize()
    {
        for ( auto& i : mTriangleGroups )
        {
            if ( glIsVertexArray ( i.mArray ) )
            {
                glDeleteVertexArrays ( 1, &i.mArray );
                i.mArray = 0;
            }
            if ( glIsBuffer ( i.mVertexBuffer ) )
            {
                glDeleteBuffers ( 1, &i.mVertexBuffer );
                i.mVertexBuffer = 0;
            }
            if ( glIsBuffer ( i.mIndexBuffer ) )
            {
                glDeleteBuffers ( 1, &i.mIndexBuffer );
                i.mIndexBuffer = 0;
            }
        }
    }

    uint32_t OpenGLMesh::GetStride ( uint32_t aFlags ) const
    {
        uint32_t stride = 0;
        if ( aFlags & POSITION_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & NORMAL_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & TANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & BITANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & UV_BIT )
        {
            stride += sizeof ( float ) * 2;
        }
        if ( aFlags & WEIGHT_BIT )
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

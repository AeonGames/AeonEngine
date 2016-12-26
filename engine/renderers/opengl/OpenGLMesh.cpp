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
    OpenGLMesh::OpenGLMesh ( const std::shared_ptr<Mesh> aMesh ) :
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
        auto& triangle_groups = mMesh->GetTriangleGroups();
        for ( size_t i = 0; i < triangle_groups.size(); ++i )
        {
            glBindVertexArray ( mBuffers[i].mArray );
            OPENGL_CHECK_ERROR_NO_THROW;
            if ( triangle_groups[i].mIndexCount )
            {
                glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mBuffers[i].mIndexBuffer );
                OPENGL_CHECK_ERROR_NO_THROW;
                glDrawElements ( GL_TRIANGLES, triangle_groups[i].mIndexCount, triangle_groups[i].mIndexType, 0 );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            else
            {
                glDrawArrays ( GL_TRIANGLES, 0, triangle_groups[i].mVertexCount );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
        }
    }

    void OpenGLMesh::Initialize()
    {
        auto& triangle_groups = mMesh->GetTriangleGroups();
        mBuffers.resize ( triangle_groups.size() );
        for ( size_t i = 0; i < triangle_groups.size(); ++i )
        {
            glGenVertexArrays ( 1, &mBuffers[i].mArray );
            OPENGL_CHECK_ERROR_THROW;
            glBindVertexArray ( mBuffers[i].mArray );
            OPENGL_CHECK_ERROR_THROW;
            glGenBuffers ( 1, &mBuffers[i].mVertexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBindBuffer ( GL_ARRAY_BUFFER, mBuffers[i].mVertexBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBufferData ( GL_ARRAY_BUFFER, triangle_groups[i].mVertexBuffer.size(), triangle_groups[i].mVertexBuffer.data(), GL_STATIC_DRAW );
            OPENGL_CHECK_ERROR_THROW;

            uint8_t* offset = nullptr;
            if ( triangle_groups[i].mVertexFlags & Mesh::POSITION_BIT )
            {
                glEnableVertexAttribArray ( 0 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride ( triangle_groups[i].mVertexFlags ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 3;
            }

            if ( triangle_groups[i].mVertexFlags & Mesh::NORMAL_BIT )
            {
                glEnableVertexAttribArray ( 1 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride ( triangle_groups[i].mVertexFlags ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 3;
            }

            if ( triangle_groups[i].mVertexFlags & Mesh::TANGENT_BIT )
            {
                glEnableVertexAttribArray ( 2 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride ( triangle_groups[i].mVertexFlags ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 3;
            }

            if ( triangle_groups[i].mVertexFlags & Mesh::BITANGENT_BIT )
            {
                glEnableVertexAttribArray ( 3 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 3, 3, GL_FLOAT, GL_FALSE, mMesh->GetStride ( triangle_groups[i].mVertexFlags ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 3;
            }

            if ( triangle_groups[i].mVertexFlags & Mesh::UV_BIT )
            {
                glEnableVertexAttribArray ( 4 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 4, 2, GL_FLOAT, GL_FALSE, mMesh->GetStride ( triangle_groups[i].mVertexFlags ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( float ) * 2;
            }

            if ( triangle_groups[i].mVertexFlags & Mesh::WEIGHT_BIT )
            {
                glEnableVertexAttribArray ( 5 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 5, 4, GL_UNSIGNED_BYTE, GL_FALSE, mMesh->GetStride ( triangle_groups[i].mVertexFlags ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( uint8_t ) * 4;
                glEnableVertexAttribArray ( 6 );
                OPENGL_CHECK_ERROR_THROW;
                glVertexAttribPointer ( 6, 4, GL_UNSIGNED_BYTE, GL_TRUE, mMesh->GetStride ( triangle_groups[i].mVertexFlags ), offset );
                OPENGL_CHECK_ERROR_THROW;
                offset += sizeof ( uint8_t ) * 4;
            }
            //---Index Buffer---
            if ( triangle_groups[i].mIndexCount )
            {
                glGenBuffers ( 1, &mBuffers[i].mIndexBuffer );
                OPENGL_CHECK_ERROR_THROW;
                glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, mBuffers[i].mIndexBuffer );
                OPENGL_CHECK_ERROR_THROW;
                glBufferData ( GL_ELEMENT_ARRAY_BUFFER, triangle_groups[i].mIndexBuffer.size(), triangle_groups[i].mIndexBuffer.data(), GL_STATIC_DRAW );
                OPENGL_CHECK_ERROR_THROW;
            }
        }
    }

    void OpenGLMesh::Finalize()
    {
        OPENGL_CHECK_ERROR_NO_THROW;
        for ( auto& i : mBuffers )
        {
            if ( glIsVertexArray ( i.mArray ) )
            {
                OPENGL_CHECK_ERROR_NO_THROW;
                glDeleteVertexArrays ( 1, &i.mArray );
                OPENGL_CHECK_ERROR_NO_THROW;
                i.mArray = 0;
            }
            OPENGL_CHECK_ERROR_NO_THROW;
            if ( glIsBuffer ( i.mVertexBuffer ) )
            {
                OPENGL_CHECK_ERROR_NO_THROW;
                glDeleteBuffers ( 1, &i.mVertexBuffer );
                OPENGL_CHECK_ERROR_NO_THROW;
                i.mVertexBuffer = 0;
            }
            OPENGL_CHECK_ERROR_NO_THROW;
            if ( glIsBuffer ( i.mIndexBuffer ) )
            {
                OPENGL_CHECK_ERROR_NO_THROW;
                glDeleteBuffers ( 1, &i.mIndexBuffer );
                OPENGL_CHECK_ERROR_NO_THROW;
                i.mIndexBuffer = 0;
            }
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }
}

/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <vector>
#include "aeongames/Skeleton.h"
#include "aeongames/ResourceCache.h"
#include "OpenGLSkeleton.h"

namespace AeonGames
{
    OpenGLSkeleton::OpenGLSkeleton ( const std::shared_ptr<const Skeleton> aSkeleton, const std::shared_ptr<const OpenGLRenderer> aOpenGLRenderer ) :
        mSkeleton ( aSkeleton )
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

    OpenGLSkeleton::~OpenGLSkeleton()
    {
        Finalize();
    }

    GLuint OpenGLSkeleton::GetBuffer() const
    {
        return mSkeletonBuffer;
    }

    void OpenGLSkeleton::Initialize()
    {
        GLint max_uniform_block_size = 0;
        glGetIntegerv ( GL_MAX_UNIFORM_BLOCK_SIZE, &max_uniform_block_size );
        GLint skeleton_size = static_cast<GLint> ( mSkeleton->GetJoints().size() * sizeof ( float ) * 16 );
        if ( skeleton_size > max_uniform_block_size )
        {
            std::ostringstream stream;
            stream << "Size of Skeleton is over maximum uniform block size: " << skeleton_size << " > " << max_uniform_block_size;
            throw std::runtime_error ( stream.str().c_str() );
        }
        glGenBuffers ( 1, &mSkeletonBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBindBuffer ( GL_UNIFORM_BUFFER, mSkeletonBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBufferData ( GL_UNIFORM_BUFFER, skeleton_size, nullptr, GL_DYNAMIC_DRAW );
        OPENGL_CHECK_ERROR_THROW;
        glBindBuffer ( GL_UNIFORM_BUFFER, mSkeletonBuffer );
        OPENGL_CHECK_ERROR_THROW;
        float* joint_array = reinterpret_cast<float*> ( glMapBuffer ( GL_UNIFORM_BUFFER, GL_WRITE_ONLY ) );
        OPENGL_CHECK_ERROR_THROW;
        const float identity[16] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        for ( size_t i = 0; i < mSkeleton->GetJoints().size(); ++i )
        {
            //mSkeleton->GetJoints() [i].GetTransform().GetInvertedMatrix ( joint_array + ( i * 16 ) );
            memcpy ( ( joint_array + ( i * 16 ) ), identity, sizeof ( float ) * 16 );
        }
        glUnmapBuffer ( GL_UNIFORM_BUFFER );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLSkeleton::Finalize()
    {
        if ( mSkeletonBuffer != 0 )
        {
            glDeleteBuffers ( 1, &mSkeletonBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mSkeletonBuffer = 0;
        }
    }
}

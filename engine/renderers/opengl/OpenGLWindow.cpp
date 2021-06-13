/*
Copyright (C) 2017-2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Frustum.h"
#include "aeongames/AABB.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Node.h"
#include "aeongames/Scene.h"
#include "aeongames/Material.h"
#include "aeongames/MemoryPool.h" ///<- This is here just for the literals
#include "OpenGLWindow.h"
#include "OpenGLRenderer.h"
#include "OpenGLFunctions.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <utility>

namespace AeonGames
{
    void OpenGLWindow::Initialize()
    {
        glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        OPENGL_CHECK_ERROR_THROW;
        glEnable ( GL_BLEND );
        OPENGL_CHECK_ERROR_THROW;
        glDepthFunc ( GL_LESS );
        OPENGL_CHECK_ERROR_THROW;
        glEnable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_THROW;
        glCullFace ( GL_BACK );
        OPENGL_CHECK_ERROR_THROW;
        glEnable ( GL_CULL_FACE );
        OPENGL_CHECK_ERROR_THROW;
        /// @todo Initial clear color should be configurable.
        glClearColor ( 0.5f, 0.5f, 0.5f, 1.0f );
        OPENGL_CHECK_ERROR_THROW;
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        OPENGL_CHECK_ERROR_THROW;
        mMatrices.Initialize ( sizeof ( float ) * 16 * 3, GL_DYNAMIC_DRAW );
    }

    void OpenGLWindow::Finalize()
    {
        //mOverlay.Finalize();
        mMatrices.Finalize();
    }

    OpenGLWindow::OpenGLWindow ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        mOpenGLRenderer { aOpenGLRenderer },
        //mOverlay{Texture::Format::RGBA, Texture::Type::UNSIGNED_INT_8_8_8_8_REV, aWidth, aHeight},
        mMemoryPoolBuffer{aOpenGLRenderer, static_cast<GLsizei> ( 8_mb ) }
    {
    }

    OpenGLWindow::OpenGLWindow ( const OpenGLRenderer&  aOpenGLRenderer ) :
        mOpenGLRenderer{ aOpenGLRenderer },
        //mOverlay{Texture::Format::RGBA, Texture::Type::UNSIGNED_INT_8_8_8_8_REV},
        mMemoryPoolBuffer{aOpenGLRenderer, static_cast<GLsizei> ( 8_mb ) }
    {
    }

    OpenGLWindow::~OpenGLWindow() = default;

    static GLenum GetIndexType ( const Mesh& aMesh )
    {
        switch ( aMesh.GetIndexSize() )
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

    static const std::unordered_map<Topology, GLenum> TopologyMap
    {
        {POINT_LIST, GL_POINTS},
        {LINE_STRIP, GL_LINE_STRIP},
        {LINE_LIST, GL_LINES},
        {TRIANGLE_STRIP, GL_TRIANGLE_STRIP},
        {TRIANGLE_FAN, GL_TRIANGLE_FAN},
        {TRIANGLE_LIST, GL_TRIANGLES},
        {LINE_LIST_WITH_ADJACENCY, GL_LINES_ADJACENCY},
        {LINE_STRIP_WITH_ADJACENCY, GL_LINE_STRIP_ADJACENCY},
        {TRIANGLE_LIST_WITH_ADJACENCY, GL_TRIANGLES_ADJACENCY},
        {TRIANGLE_STRIP_WITH_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY},
        {PATCH_LIST, GL_PATCHES},
    };

    void OpenGLWindow::Render ( const Matrix4x4& aModelMatrix,
                                const Mesh& aMesh,
                                const Pipeline& aPipeline,
                                const Material* aMaterial,
                                const BufferAccessor* aSkeleton,
                                uint32_t aVertexStart,
                                uint32_t aVertexCount,
                                uint32_t aInstanceCount,
                                uint32_t aFirstInstance ) const
    {
        mOpenGLRenderer.BindPipeline ( aPipeline, aMaterial, aSkeleton );
        OPENGL_CHECK_ERROR_NO_THROW;

        mMatrices.WriteMemory ( 0, sizeof ( float ) * 16, aModelMatrix.GetMatrix4x4() );

        glBindBuffer ( GL_UNIFORM_BUFFER, mMatrices.GetBufferId() );
        OPENGL_CHECK_ERROR_THROW;
        glBindBufferBase ( GL_UNIFORM_BUFFER, 0, mMatrices.GetBufferId() );
        OPENGL_CHECK_ERROR_THROW;

        /// @todo Add some sort of way to make use of the aFirstInstance parameter
        mOpenGLRenderer.BindMesh ( aMesh );
        if ( aMesh.GetIndexCount() )
        {
            glDrawElementsInstanced ( TopologyMap.at ( aPipeline.GetTopology() ), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                      GetIndexType ( aMesh ), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glDrawArraysInstanced ( TopologyMap.at ( aPipeline.GetTopology() ), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    const GLuint OpenGLWindow::GetMatricesBuffer() const
    {
        return mMatrices.GetBufferId();
    }

    BufferAccessor OpenGLWindow::AllocateSingleFrameUniformMemory ( size_t aSize )
    {
        return mMemoryPoolBuffer.Allocate ( aSize );
    }

    void OpenGLWindow::BeginRender()
    {
        MakeCurrent();
        mFrameBuffer.Bind();
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glEnable ( GL_DEPTH_TEST );
    }

    void OpenGLWindow::EndRender()
    {
        mFrameBuffer.Unbind();
        glClear ( GL_COLOR_BUFFER_BIT );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDisable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_NO_THROW;
        GLint dims[4] = {0};
        glGetIntegerv ( GL_VIEWPORT, dims );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBlitNamedFramebuffer (
            mFrameBuffer.GetFBO(),
            0,
            dims[0],
            dims[1],
            dims[2],
            dims[3],
            dims[0],
            dims[1],
            dims[2],
            dims[3],
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR );
        OPENGL_CHECK_ERROR_NO_THROW;
#if 0
        /* Bind and render overlay texture */
        glUseProgram ( mOpenGLRenderer.GetOverlayProgram() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBuffer ( GL_ARRAY_BUFFER, mOpenGLRenderer.GetOverlayQuad() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindTexture ( GL_TEXTURE_2D, mOverlay.GetTextureId() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnableVertexAttribArray ( 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glVertexAttribPointer ( 0, 2, GL_FLOAT, GL_FALSE, sizeof ( float ) * 4, 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnableVertexAttribArray ( 1 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glVertexAttribPointer ( 1, 2, GL_FLOAT, GL_FALSE, sizeof ( float ) * 4, reinterpret_cast<const void*> ( sizeof ( float ) * 2 ) );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDrawArrays ( GL_TRIANGLE_FAN, 0, 4 );
        OPENGL_CHECK_ERROR_NO_THROW;
#endif
        SwapBuffers();
        mMemoryPoolBuffer.Reset();
    }
    void OpenGLWindow::WriteOverlayPixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Texture::Format aFormat, Texture::Type aType, const uint8_t* aPixels )
    {
        //mOverlay.WritePixels ( aXOffset, aYOffset, aWidth, aHeight, aFormat, aType, aPixels );
    }

    void OpenGLWindow::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mProjectionMatrix = aMatrix * Matrix4x4
        {
            // Flip Matrix
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        mFrustum = mProjectionMatrix * mViewMatrix;
        mMatrices.WriteMemory ( sizeof ( float ) * 16, sizeof ( float ) * 16, mProjectionMatrix.GetMatrix4x4() );
    }
    void OpenGLWindow::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mViewMatrix = aMatrix;
        mFrustum = mProjectionMatrix * mViewMatrix;
        mMatrices.WriteMemory ( sizeof ( float ) * 16 * 2, sizeof ( float ) * 16, mViewMatrix.GetMatrix4x4() );
    }

    const Matrix4x4 & OpenGLWindow::GetProjectionMatrix() const
    {
        return mProjectionMatrix;
    }
    const Matrix4x4 & OpenGLWindow::GetViewMatrix() const
    {
        return mViewMatrix;
    }

    void OpenGLWindow::ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        mAspectRatio = ( static_cast<float> ( aWidth ) / static_cast<float> ( aHeight ) );
        if ( aWidth && aHeight )
        {
            MakeCurrent();
            OPENGL_CHECK_ERROR_NO_THROW;
            glViewport ( aX, aY, aWidth, aHeight );
            OPENGL_CHECK_ERROR_THROW;
            mFrameBuffer.Resize ( aWidth, aHeight );
            //mOverlay.Resize ( aWidth, aHeight );
        }
    }
}

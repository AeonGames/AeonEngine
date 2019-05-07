/*
Copyright (C) 2017-2019 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLWindow.h"
#include "OpenGLRenderer.h"
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLMesh.h"
#include "OpenGLFunctions.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <utility>

namespace AeonGames
{
    void OpenGLWindow::InitializeCommon()
    {
#ifdef SINGLE_VAO
        glGenVertexArrays ( 1, &mVAO );
        OPENGL_CHECK_ERROR_THROW;
        glBindVertexArray ( mVAO );
        OPENGL_CHECK_ERROR_THROW;
#endif
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

        // Initialize Matrix Buffer
        glGenBuffers ( 1, &mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        float matrices[48] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glBufferData ( GL_UNIFORM_BUFFER, sizeof ( float ) * 48,
                       matrices, GL_DYNAMIC_DRAW );
        OPENGL_CHECK_ERROR_NO_THROW;

    }

    void OpenGLWindow::FinalizeCommon()
    {
        if ( glIsBuffer ( mMatricesBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteBuffers ( 1, &mMatricesBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mMatricesBuffer = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
#ifdef SINGLE_VAO
        if ( glIsVertexArray ( mVAO ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteVertexArrays ( 1, &mVAO );
            OPENGL_CHECK_ERROR_NO_THROW;
            mVAO = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
#endif
    }

    OpenGLWindow::OpenGLWindow ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        Window{aX, aY, aWidth, aHeight, aFullScreen}, mOpenGLRenderer ( aOpenGLRenderer ), mOwnsWindowId{ true }, mFullScreen{aFullScreen}
    {
        try
        {
            InitializePlatform();
            InitializeCommon();
        }
        catch ( ... )
        {
            FinalizeCommon();
            FinalizePlatform();
            throw;
        }
    }
    OpenGLWindow::OpenGLWindow ( const OpenGLRenderer&  aOpenGLRenderer, void* aWindowId ) :
        Window{aWindowId}, mOpenGLRenderer ( aOpenGLRenderer )
    {
        try
        {
            InitializePlatform();
            InitializeCommon();
        }
        catch ( ... )
        {
            FinalizeCommon();
            FinalizePlatform();
            throw;
        }
    }

    OpenGLWindow::~OpenGLWindow()
    {
        FinalizeCommon();
        FinalizePlatform();
    }

    void* OpenGLWindow::GetWindowId() const
    {
        return mWindowId;
    }

    void OpenGLWindow::Render ( const Matrix4x4& aModelMatrix,
                                const Mesh& aMesh,
                                const Pipeline& aPipeline,
                                const Material* aMaterial,
                                const UniformBuffer* aSkeleton,
                                uint32_t aVertexStart,
                                uint32_t aVertexCount,
                                uint32_t aInstanceCount,
                                uint32_t aFirstInstance ) const
    {
        const OpenGLMesh& opengl_mesh{reinterpret_cast<const OpenGLMesh&> ( aMesh ) };
        const OpenGLPipeline& opengl_pipeline{reinterpret_cast<const OpenGLPipeline&> ( aPipeline ) };
        opengl_pipeline.Use (
            reinterpret_cast<const OpenGLMaterial&> ( ( aMaterial ) ?
                    *aMaterial : opengl_pipeline.GetDefaultMaterial() ),
            reinterpret_cast<const OpenGLUniformBuffer*> ( aSkeleton ) );
        OPENGL_CHECK_ERROR_NO_THROW;
        glNamedBufferSubData ( mMatricesBuffer, ( sizeof ( float ) * 16 ) * 0, sizeof ( float ) * 16, aModelMatrix.GetMatrix4x4() );
        OPENGL_CHECK_ERROR_NO_THROW;

        /// @todo Add some sort of way to make use of the aFirstInstance parameter
        opengl_mesh.BindVertexArray();
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( aMesh.GetIndexCount() )
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, opengl_mesh.GetIndexBufferId() );
            OPENGL_CHECK_ERROR_NO_THROW;
            glDrawElementsInstanced ( opengl_pipeline.GetTopology(), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                      opengl_mesh.GetIndexType(), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glDrawArraysInstanced ( opengl_pipeline.GetTopology(), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    const GLuint OpenGLWindow::GetMatricesBuffer() const
    {
        return mMatricesBuffer;
    }
}

/*
Copyright (C) 2016-2017 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "math/3DMath.h"
#include "OpenGLFunctions.h"
#include "OpenGLRenderer.h"
#include "OpenGLWindow.h"
#include "OpenGLMesh.h"
#include "OpenGLPipeline.h"
#include "OpenGLTexture.h"
#include "OpenGLModel.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Window.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/Model.h"
#include "aeongames/ModelInstance.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/Transform.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"
#include "aeongames/Frustum.h"

namespace AeonGames
{
    OpenGLRenderer::OpenGLRenderer()
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

    OpenGLRenderer::~OpenGLRenderer()
    {
        Finalize();
    }

    void OpenGLRenderer::Render ( const std::shared_ptr<const Scene>& aScene ) const
    {
        Frustum frustum ( mProjectionMatrix * mViewTransform.GetInverted().GetMatrix() );

        aScene->LoopTraverseDFSPreOrder ( [this, &frustum] ( const std::shared_ptr<const Node>& aNode )
        {
            const std::unique_ptr<RenderModel>& render_model = GetRenderModel ( aNode->GetModelInstance()->GetModel() );
            if ( render_model )
            {
                if ( frustum.Intersects ( aNode->GetGlobalAABB() ) )
                {
                    render_model->Render ( aNode->GetModelInstance() );
                }
            }
            else
            {
                /* This is lazy loading */
                SetRenderModel ( aNode->GetModelInstance()->GetModel(), std::make_unique<OpenGLModel> ( aNode->GetModelInstance()->GetModel(), shared_from_this() ) );
            }
        } );
    }

    void OpenGLRenderer::CacheScene ( const std::shared_ptr<const Scene>& aScene ) const
    {
        aScene->LoopTraverseDFSPreOrder ( [this] ( const std::shared_ptr<const Node>& aNode )
        {
            if ( !GetRenderModel ( aNode->GetModelInstance()->GetModel() ) )
            {
                SetRenderModel ( aNode->GetModelInstance()->GetModel(), std::make_unique<OpenGLModel> ( aNode->GetModelInstance()->GetModel(), shared_from_this() ) );
            }
        } );
    }

    void* OpenGLRenderer::GetWindowId() const
    {
        return mWindowId;
    }

    GLuint OpenGLRenderer::GetMatricesBuffer() const
    {
        return mMatricesBuffer;
    }

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowProxy ( void * aWindowId ) const
    {
        return std::make_unique<OpenGLWindow> ( aWindowId, shared_from_this() );
    }

    void OpenGLRenderer::SetViewTransform ( const Transform aTransform )
    {
        mViewTransform = aTransform;
        Matrix4x4 view_matrix{ aTransform.GetInvertedMatrix() };
        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, sizeof ( float ) * 16, sizeof ( float ) * 16, view_matrix.GetMatrix4x4() );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLRenderer::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mProjectionMatrix =
            aMatrix * Matrix4x4
        {
            // Flip Matrix
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( float ) * 16, ( mProjectionMatrix ).GetMatrix4x4() );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void* OpenGLRenderer::GetOpenGLContext() const
    {
        return mOpenGLContext;
    }
}

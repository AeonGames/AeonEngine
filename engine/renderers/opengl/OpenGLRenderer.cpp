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

    void OpenGLRenderer::Render ( const std::shared_ptr<const ModelInstance>& aModelInstance ) const
    {
        try
        {
            mModelLibrary.at ( aModelInstance->GetModel()->GetFilename() )->Render ( aModelInstance );
        }
        catch ( std::out_of_range& e )
        {
            std::cout << "Model " << aModelInstance->GetModel()->GetFilename() << " Not loaded into renderer (" << e.what() << ")" << std::endl;
        }
    }

    void OpenGLRenderer::LoadModel ( const std::shared_ptr<const Model>& aModel )
    {
        mModelLibrary[aModel->GetFilename()] = std::make_unique<OpenGLModel> ( aModel, shared_from_this() );
    }

    void OpenGLRenderer::UnloadModel ( const std::shared_ptr<const Model>& aModel )
    {
        mModelLibrary.erase ( aModel->GetFilename() );
    }

    void OpenGLRenderer::LoadScene ( const std::shared_ptr<const Scene>& aScene )
    {
    }

    void OpenGLRenderer::UnloadScene ( const std::shared_ptr<const Scene>& aScene )
    {
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
        aTransform.GetInvertedMatrix ( mViewMatrix );
        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( mMatrices ), mMatrices );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLRenderer::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        memcpy ( mProjectionMatrix, aMatrix.GetMatrix4x4(),  sizeof ( float ) * 16 );
        /* Flip Z axis to match Vulkan's right hand Normalized Device Coordinates (NDC).*/
        mProjectionMatrix[8]  *= -1;
        mProjectionMatrix[9]  *= -1;
        mProjectionMatrix[10] *= -1;
        mProjectionMatrix[11] *= -1;
        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( mMatrices ), mMatrices );
        OPENGL_CHECK_ERROR_NO_THROW;
    }
#if 0
    void OpenGLRenderer::SetModelMatrix ( const float aMatrix[16] )
    {
        memcpy ( mModelMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }
#endif
    void* OpenGLRenderer::GetOpenGLContext() const
    {
        return mOpenGLContext;
    }
}

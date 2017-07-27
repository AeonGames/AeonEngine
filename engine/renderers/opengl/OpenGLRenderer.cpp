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
#if 0
    void OpenGLRenderer::BeginRender ( void* aWindowId ) const
    {
        auto i = std::find ( mWindowRegistry.begin(), mWindowRegistry.end(), aWindowId );
        if ( i != mWindowRegistry.end() )
        {
#ifdef _WIN32
            HDC hdc = GetDC ( reinterpret_cast<HWND> ( *i ) );
            wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLContext ) );
            glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            ReleaseDC ( reinterpret_cast<HWND> ( *i  ), hdc );
#else
            glXMakeCurrent ( static_cast<Display*> ( mWindowId ),
                             reinterpret_cast<Window> ( *i ),
                             static_cast<GLXContext> ( mOpenGLContext ) );
            glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#endif
        }
    }

    void OpenGLRenderer::Render ( void* aWindowId, const std::shared_ptr<Model> aModel ) const
    {
        auto& model = mModelMap.at ( aModel );
        model->Render ( mMatricesBuffer );
    }
    void OpenGLRenderer::EndRender ( void* aWindowId ) const
    {
        auto i = std::find ( mWindowRegistry.begin(), mWindowRegistry.end(), aWindowId );
        if ( i != mWindowRegistry.end() )
        {
#if _WIN32
            HDC hdc = GetDC ( reinterpret_cast<HWND> ( *i ) );
            wglMakeCurrent ( hdc, reinterpret_cast<HGLRC> ( mOpenGLContext ) );
            SwapBuffers ( hdc );
            ReleaseDC ( reinterpret_cast<HWND> ( *i ), hdc );
#else
            glXMakeCurrent ( static_cast<Display*> ( mWindowId ),
                             reinterpret_cast<Window> ( *i ),
                             static_cast<GLXContext> ( mOpenGLContext ) );
            glXSwapBuffers ( static_cast<Display*> ( mWindowId ),
                             reinterpret_cast<Window> ( *i ) );
#endif
        }
    }
#endif
    bool OpenGLRenderer::AllocateModelRenderData ( std::shared_ptr<Model> aModel )
    {
        if ( mModelMap.find ( aModel ) == mModelMap.end() )
        {
            /* We dont really need to cache OpenGL Models,
            since mModelMap IS our model cache.
            We DO need a deallocation function.*/
            mModelMap[aModel] = std::make_unique<OpenGLModel> ( aModel );
        }
        return true;
    }

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowProxy ( void * aWindowId )
    {
        return std::make_unique<OpenGLWindow> ( shared_from_this(), aWindowId );
    }

    void OpenGLRenderer::SetViewMatrix ( const float aMatrix[16] )
    {
        memcpy ( mViewMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }

    void OpenGLRenderer::SetProjectionMatrix ( const float aMatrix[16] )
    {
        memcpy ( mProjectionMatrix, aMatrix,  sizeof ( float ) * 16 );
        /* Flip Z axis to match Vulkan's right hand Normalized Device Coordinates (NDC).*/
        mProjectionMatrix[8]  *= -1;
        mProjectionMatrix[9]  *= -1;
        mProjectionMatrix[10] *= -1;
        mProjectionMatrix[11] *= -1;
        UpdateMatrices();
    }

    void OpenGLRenderer::SetModelMatrix ( const float aMatrix[16] )
    {
        memcpy ( mModelMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }

    void* OpenGLRenderer::GetOpenGLContext() const
    {
        return mOpenGLContext;
    }

    void OpenGLRenderer::UpdateMatrices()
    {
        /** @todo Either publish this function or
            add arguments so just some matrices are
            updated based on which one changed.*/
        // Update mViewProjectionMatrix
        Multiply4x4Matrix ( mProjectionMatrix, mViewMatrix, mViewProjectionMatrix );
        // Update mModelViewMatrix
        Multiply4x4Matrix ( mViewMatrix, mModelMatrix, mModelViewMatrix );
        // Update mModelViewProjectionMatrix
        Multiply4x4Matrix ( mViewProjectionMatrix, mModelMatrix, mModelViewProjectionMatrix );
        /*  Calculate Normal Matrix
            Inverting a 3x3 matrix is cheaper than inverting a 4x4 matrix,
            so even if the shader alignment requires us to pad the 3x3 matrix into
            a 4x3 matrix we do these operations on a 3x3 basis.*/
        Extract3x3Matrix ( mModelViewMatrix, mNormalMatrix );
        Invert3x3Matrix ( mNormalMatrix, mNormalMatrix );
        Transpose3x3Matrix ( mNormalMatrix, mNormalMatrix );
        Convert3x3To4x3 ( mNormalMatrix, mNormalMatrix );

        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( mMatrices ), mMatrices );
        OPENGL_CHECK_ERROR_NO_THROW;
    }
}

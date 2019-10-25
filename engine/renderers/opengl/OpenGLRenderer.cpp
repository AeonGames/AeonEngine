/*
Copyright (C) 2016-2019 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLRenderer.h"
#include "OpenGLImage.h"
#include "OpenGLMesh.h"
#include "OpenGLMaterial.h"
#include "OpenGLBuffer.h"
#include "OpenGLX11Window.h"
#include "OpenGLWinWindow.h"

namespace AeonGames
{
    OpenGLRenderer::OpenGLRenderer()
    {
        try
        {
            Initialize();
            if ( !LoadOpenGLAPI() )
            {
                throw std::runtime_error ( "Unable to Load OpenGL functions." );
            }
            /** @todo load mOverlay pipeline */
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

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowProxy ( void * aWindowId ) const
    {
        return std::make_unique<OpenGLPlatformWindow> ( *this, aWindowId );
    }

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowInstance ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) const
    {
        return std::make_unique<OpenGLPlatformWindow> ( *this, aX, aY, aWidth, aHeight, aFullScreen );
    }

    std::unique_ptr<Mesh> OpenGLRenderer::CreateMesh ( uint32_t aPath ) const
    {
        return std::make_unique<OpenGLMesh> ( aPath );
    }

    std::unique_ptr<Pipeline> OpenGLRenderer::CreatePipeline ( uint32_t aPath ) const
    {
        return std::make_unique<OpenGLPipeline> ( aPath );
    }

    std::unique_ptr<Material> OpenGLRenderer::CreateMaterial ( uint32_t aPath ) const
    {
        return std::make_unique<OpenGLMaterial> ( aPath );
    }

    std::unique_ptr<Image> OpenGLRenderer::CreateImage ( uint32_t aPath ) const
    {
        return std::make_unique<OpenGLImage> ( aPath );
    }

    std::unique_ptr<Buffer> OpenGLRenderer::CreateBuffer ( size_t aSize, const void* aData ) const
    {
        return std::make_unique<OpenGLBuffer> ( static_cast<GLsizei> ( aSize ), GL_DYNAMIC_DRAW, aData );
    }

    void* OpenGLRenderer::GetOpenGLContext() const
    {
        return mOpenGLContext;
    }

    void* OpenGLRenderer::GetDeviceContext() const
    {
        return mDeviceContext;
    }

    void* OpenGLRenderer::GetWindowId() const
    {
        return mWindowId;
    }
}

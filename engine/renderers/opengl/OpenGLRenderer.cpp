/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLWindow.h"
#include "OpenGLMesh.h"
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLImage.h"
#include "OpenGLBuffer.h"
#include "OpenGLUniformBuffer.h"

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

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowProxy ( void * aWindowId ) const
    {
        return std::make_unique<OpenGLWindow> ( aWindowId, *this );
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

    std::unique_ptr<UniformBuffer> OpenGLRenderer::CreateUniformBuffer ( size_t aSize, const void* aData ) const
    {
        return std::make_unique<OpenGLUniformBuffer> ( static_cast<GLsizei> ( aSize ) );
    }

    void* OpenGLRenderer::GetOpenGLContext() const
    {
        return mOpenGLContext;
    }

    void* OpenGLRenderer::GetWindowId() const
    {
        return mWindowId;
    }
}

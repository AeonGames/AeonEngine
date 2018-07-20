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
#include "OpenGLMaterial.h"
#include "OpenGLPipeline.h"

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

    void OpenGLRenderer::LoadRenderMesh ( const Mesh & aMesh ) const
    {
        aMesh.SetRenderMesh ( std::make_unique<OpenGLMesh> ( aMesh, *this ) );
    }

    void OpenGLRenderer::UnloadRenderMesh ( const Mesh & aMesh ) const
    {
        aMesh.SetRenderMesh ( nullptr );
    }

    void OpenGLRenderer::LoadRenderPipeline ( const Pipeline & aPipeline ) const
    {
        aPipeline.SetRenderPipeline ( std::make_unique<OpenGLPipeline> ( aPipeline, *this ) );
    }

    void OpenGLRenderer::UnloadRenderPipeline ( const Pipeline & aPipeline ) const
    {
        aPipeline.SetRenderPipeline ( nullptr );
    }

    void OpenGLRenderer::LoadRenderMaterial ( const Material & aMaterial ) const
    {
        aMaterial.SetRenderMaterial ( std::make_unique<OpenGLMaterial> ( aMaterial ) );
    }

    void OpenGLRenderer::UnloadRenderMaterial ( const Material & aMaterial ) const
    {
        aMaterial.SetRenderMaterial ( nullptr );
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

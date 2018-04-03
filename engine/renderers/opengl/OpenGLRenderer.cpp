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

    void* OpenGLRenderer::GetWindowId() const
    {
        return mWindowId;
    }

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowProxy ( void * aWindowId ) const
    {
        return std::make_unique<OpenGLWindow> ( aWindowId, shared_from_this() );
    }

#if 0
    // To be refactored.
    void OpenGLRenderer::Render ( const Node& aNode, const Matrix4x4& aProjectionMatrix, const Matrix4x4& aViewMatrix ) const
    {
        const auto* model_instance = reinterpret_cast<const ModelInstance*> ( aNode.GetComponent ( ModelInstance::TypeId ) );
        if ( model_instance )
        {
            mOpenGLModels.at ( model_instance->GetModel().get() )->Render ( model_instance, aProjectionMatrix, aViewMatrix );
        }
    }

    void OpenGLRenderer::Load ( const Node& aNode )
    {
        const auto* model_instance = reinterpret_cast<const ModelInstance*> ( aNode.GetComponent ( ModelInstance::TypeId ) );
        if ( model_instance )
        {
            mOpenGLModels[model_instance->GetModel().get()] = std::make_shared<OpenGLModel> ( model_instance->GetModel(), shared_from_this() );
        }
    }

    void OpenGLRenderer::Unload ( const Node& aNode )
    {
        const auto* model_instance = reinterpret_cast<const ModelInstance*> ( aNode.GetComponent ( ModelInstance::TypeId ) );
        if ( model_instance )
        {
            mOpenGLModels.erase ( model_instance->GetModel().get() );
        }
    }
#endif
    void* OpenGLRenderer::GetOpenGLContext() const
    {
        return mOpenGLContext;
    }
}

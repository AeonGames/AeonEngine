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
#include <cassert>
#include <vector>
#include "aeongames/Model.h"
#include "aeongames/ResourceCache.h"
#include "OpenGLRenderer.h"
#include "OpenGLModel.h"
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLMesh.h"
#include "OpenGLSkeleton.h"

namespace AeonGames
{
    OpenGLModel::OpenGLModel ( const std::shared_ptr<const Model> aModel, const std::shared_ptr<const OpenGLRenderer> aOpenGLRenderer ) :
        mModel ( aModel ), mOpenGLRenderer ( aOpenGLRenderer )
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

    OpenGLModel::~OpenGLModel()
    {
        Finalize();
    }

    void OpenGLModel::Render ( size_t aAnimationIndex, float aTime ) const
    {
        for ( auto& i : mMeshes )
        {
            std::get<0> ( i )->Use ( std::get<1> ( i ) );
            glBindBufferBase ( GL_UNIFORM_BUFFER, 0, mOpenGLRenderer->GetMatricesBuffer() );
            OPENGL_CHECK_ERROR_NO_THROW;
            if ( mSkeleton )
            {
                if ( aAnimationIndex < mModel->GetAnimations().size() )
                {
                    mSkeleton->SetPose ( mModel->GetAnimations() [aAnimationIndex], aTime );
                }
                glBindBufferBase ( GL_UNIFORM_BUFFER, 2, mSkeleton->GetBuffer() );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            std::get<2> ( i )->Render();
        }
    }

    void OpenGLModel::Initialize()
    {
        auto& meshes = mModel->GetMeshes();
        mMeshes.reserve ( meshes.size() );
        for ( auto& i : meshes )
        {
            mMeshes.emplace_back (
                Get<OpenGLPipeline> ( std::get<0> ( i ).get(), std::get<0> ( i ), mOpenGLRenderer ),
                std::get<1> ( i ) ? Get<OpenGLMaterial> ( std::get<1> ( i ).get(), std::get<1> ( i ), mOpenGLRenderer ) : nullptr,
                Get<OpenGLMesh> ( std::get<2> ( i ).get(), std::get<2> ( i ), mOpenGLRenderer ) );
        }
        if ( mModel->GetSkeleton() != nullptr )
        {
            mSkeleton = Get<OpenGLSkeleton> ( mModel->GetSkeleton().get(), mModel->GetSkeleton(), mOpenGLRenderer );
        }
    }

    void OpenGLModel::Finalize()
    {
    }
}

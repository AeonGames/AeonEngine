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
#include <cassert>
#include <vector>
#include "aeongames/Model.h"
#include "aeongames/ModelInstance.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/CRC.h"
#include "OpenGLRenderer.h"
#include "OpenGLModel.h"
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLMesh.h"
#include "OpenGLSkeleton.h"
#include "OpenGLWindow.h"

namespace AeonGames
{
    const size_t OpenGLModel::TypeId = "OpenGLModel"_id;
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

    void OpenGLModel::Render ( const ModelInstance* aInstance, const Matrix4x4& aProjectionMatrix, const Matrix4x4& aViewMatrix ) const
    {
        if ( mSkeleton && mModel.get() == aInstance->GetModel().get() )
        {
            mSkeleton->SetPose ( aInstance->GetSkeletonAnimation() );
        }
        for ( size_t i = 0; i < mAssemblies.size(); ++i )
        {
            if ( !aInstance->IsAssemblyEnabled ( i ) )
            {
                continue;
            }
            std::get<0> ( mAssemblies[i] )->Use ( std::get<1> ( mAssemblies[i] ) );
            OPENGL_CHECK_ERROR_NO_THROW;
            if ( mSkeleton && mModel.get() == aInstance->GetModel().get() )
            {
                glBindBufferBase ( GL_UNIFORM_BUFFER, 2, mSkeleton->GetBuffer() );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            std::get<2> ( mAssemblies[i] )->Render();
        }
    }

    void OpenGLModel::Initialize()
    {
        auto& meshes = mModel->GetMeshes();
        mAssemblies.reserve ( meshes.size() );
        for ( auto& i : meshes )
        {
            mAssemblies.emplace_back (
                Get<OpenGLPipeline> ( std::get<0> ( i ).get(), std::get<0> ( i ), mOpenGLRenderer ),
                std::get<1> ( i ) ? Get<OpenGLMaterial> ( std::get<1> ( i ).get(), std::get<1> ( i ), mOpenGLRenderer ) : nullptr,
                Get<OpenGLMesh> ( std::get<2> ( i ).get(), std::get<2> ( i ), mOpenGLRenderer ) );
        }
        if ( mModel->GetSkeleton() != nullptr )
        {
            mSkeleton = Get<OpenGLSkeleton> ( mModel->GetSkeleton().get(), mModel->GetSkeleton(), mOpenGLRenderer );
        }
    }

    void OpenGLModel::Update ( const Node& aNode, double aDelta )
    {
        ///@todo Add code to update skeleton UBO
    }

    void OpenGLModel::Finalize()
    {
    }
}

/*
Copyright (C) 2017-2020 Rodrigo Jose Hernandez Cordoba

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
#include <stdexcept>
#include <iostream>
#include "aeongames/CommonWindow.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"
namespace AeonGames
{
    CommonWindow::CommonWindow ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        mAspectRatio {static_cast<float> ( aWidth ) / static_cast<float> ( aHeight ) }, mFullScreen{aFullScreen} {}
    CommonWindow::CommonWindow() = default;
    CommonWindow::~CommonWindow() = default;
    float CommonWindow::GetAspectRatio() const
    {
        return mAspectRatio;
    }
    const Frustum& CommonWindow::GetFrustum() const
    {
        return mFrustum;
    }
    void CommonWindow::SetScene ( const Scene* aScene )
    {
        mScene = aScene;
    }

    void CommonWindow::Render ( const Scene& aScene ) const
    {
        aScene.LoopTraverseDFSPreOrder ( [this] ( const Node & aNode )
        {
            AABB transformed_aabb = aNode.GetGlobalTransform() * aNode.GetAABB();
            if ( mFrustum.Intersects ( transformed_aabb ) )
            {
                // Call Node specific rendering function.
                aNode.Render ( *this );
            }
        } );
    }

    void CommonWindow::RenderLoop()
    {
        if ( mScene == nullptr )
        {
            return;
        }
        if ( const Node* camera = mScene->GetCamera() )
        {
            SetViewMatrix ( camera->GetGlobalTransform().GetInverted().GetMatrix() );
            Matrix4x4 projection {};
            projection.Perspective ( mScene->GetFieldOfView(), GetAspectRatio(), mScene->GetNear(), mScene->GetFar() );
            SetProjectionMatrix ( projection );
        }
        BeginRender();
        Render ( *mScene );
        EndRender();
    }

    void CommonWindow::Render (
        const Transform& aModelTransform,
        const Mesh& aMesh,
        const Pipeline& aPipeline,
        const Material* aMaterial,
        const BufferAccessor* aSkeleton,
        uint32_t aVertexStart,
        uint32_t aVertexCount,
        uint32_t aInstanceCount,
        uint32_t aFirstInstance ) const
    {
        Render ( aModelTransform.GetMatrix(),
                 aMesh,
                 aPipeline,
                 aMaterial,
                 aSkeleton,
                 aVertexStart,
                 aVertexCount,
                 aInstanceCount,
                 aFirstInstance );
    }
}

/*
Copyright (C) 2017-2019 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Window.h"
#include "aeongames/Platform.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/Frustum.h"

namespace AeonGames
{
    Window::Window ( void* aWindowId ) : mWindowId{aWindowId} {}

    void Window::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mProjectionMatrix = aMatrix;
        mFrustum = mProjectionMatrix * mViewMatrix;
        OnSetProjectionMatrix();
    }
    void Window::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mViewMatrix = aMatrix;
        mFrustum = mProjectionMatrix * mViewMatrix;
        OnSetViewMatrix();
    }
    const Matrix4x4 & Window::GetProjectionMatrix() const
    {
        return mProjectionMatrix;
    }
    const Matrix4x4 & Window::GetViewMatrix() const
    {
        return mViewMatrix;
    }
    float Window::GetAspectRatio() const
    {
        return mAspectRatio;
    }
    const Frustum& Window::GetFrustum() const
    {
        return mFrustum;
    }

    void Window::ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        mAspectRatio = ( static_cast<float> ( aWidth ) / static_cast<float> ( aHeight ) );
        OnResizeViewport ( aX, aY, aWidth, aHeight );
    }

    void Window::Render ( const Scene& aScene ) const
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
    void Window::Render (
        const Transform& aModelTransform,
        const Mesh& aMesh,
        const Pipeline& aPipeline,
        const Material* aMaterial,
        const Buffer* aSkeleton,
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

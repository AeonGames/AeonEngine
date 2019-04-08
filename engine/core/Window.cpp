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
    DLL Window::~Window() = default;
    void Window::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mProjectionMatrix = aMatrix;
    }
    void Window::SetViewTransform ( const Transform& aTransform )
    {
        mViewTransform = aTransform;
    }
    const Matrix4x4 & Window::GetProjectionMatrix() const
    {
        return mProjectionMatrix;
    }
    const Transform & Window::GetViewTransform() const
    {
        return mViewTransform;
    }
    float Window::GetHalfAspectRatio() const
    {
        return mHalfAspectRatio;
    }
    void Window::ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        mHalfAspectRatio = ( static_cast<float> ( aWidth ) / static_cast<float> ( aHeight ) ) / 2.0f;
        mProjectionMatrix.Frustum ( -mHalfAspectRatio, mHalfAspectRatio, 0.5, -0.5, 1, 1600 );
        OnResizeViewport ( aX, aY, aWidth, aHeight );
    }

    void Window::Render ( const Scene& aScene ) const
    {
        Matrix4x4 view_matrix { mViewTransform.GetInverted().GetMatrix() };
        Frustum frustum ( mProjectionMatrix * view_matrix );
        aScene.LoopTraverseDFSPreOrder ( [this, &frustum] ( const Node & aNode )
        {
            AABB transformed_aabb = aNode.GetGlobalTransform() * aNode.GetAABB();
            if ( frustum.Intersects ( transformed_aabb ) )
            {
                // Call Node specific rendering function.
                aNode.Render ( *this );
            }
        } );
    }
}

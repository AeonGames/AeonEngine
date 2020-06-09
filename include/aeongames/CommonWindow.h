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
#ifndef AEONGAMES_COMMONWINDOW_H
#define AEONGAMES_COMMONWINDOW_H
#include <memory>
#include <unordered_map>
#include "aeongames/Matrix4x4.h"
#include "aeongames/Transform.h"
#include "aeongames/Frustum.h"
#include "aeongames/BufferAccessor.h"
#include "aeongames/Texture.h"
#include "aeongames/Window.h"
namespace AeonGames
{
    class Scene;
    class CommonWindow : public Window
    {
    public:
        DLL CommonWindow ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        DLL CommonWindow ();
        DLL virtual ~CommonWindow() = 0;
        DLL float GetAspectRatio() const  final;
        DLL const Frustum& GetFrustum() const  final;
        DLL void SetScene ( const Scene* aScene )  final;
        DLL void Render (
            const Transform& aModelTransform,
            const Mesh& aMesh,
            const Pipeline& aPipeline,
            const Material* aMaterial,
            const BufferAccessor* aSkeleton,
            uint32_t aVertexStart,
            uint32_t aVertexCount,
            uint32_t aInstanceCount,
            uint32_t aFirstInstance ) const final;
        virtual void Render (
            const Matrix4x4& aModelMatrix,
            const Mesh& aMesh,
            const Pipeline& aPipeline,
            const Material* aMaterial = nullptr,
            const BufferAccessor* aSkeleton = nullptr,
            uint32_t aVertexStart = 0,
            uint32_t aVertexCount = 0xffffffff,
            uint32_t aInstanceCount = 1,
            uint32_t aFirstInstance = 0 ) const = 0;
        DLL void Render ( const Scene& aScene ) const final;
        DLL void RenderLoop() final;
    protected:
        Frustum mFrustum{};
        float mAspectRatio{1.0f};
        /* @todo mScene may be a unique_ptr,
            passing ownership from where ever it came
            to the window and back,
            that way if it gets deleted ouside the window
            we won't have a lingering pointer here.*/
        const Scene* mScene{nullptr};
        bool mFullScreen{ false };
    };
}
#endif

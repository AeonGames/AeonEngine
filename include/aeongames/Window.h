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
#ifndef AEONGAMES_WINDOW_H
#define AEONGAMES_WINDOW_H
#include <memory>
#include "aeongames/Matrix4x4.h"
#include "aeongames/Transform.h"
#include "aeongames/Frustum.h"
#include "aeongames/BufferAccessor.h"
#include "aeongames/Texture.h"

namespace AeonGames
{
    class Scene;
    class Buffer;
    class Mesh;
    class Pipeline;
    class Material;
    class StringId;
    class Window
    {
    public:
        DLL Window ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        DLL Window ( void* aWindowId );
        DLL virtual ~Window() = 0;
        DLL void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight );
        DLL void Run ( Scene& aScene );
        ///@name Render Functions
        ///@{
        virtual void BeginRender() = 0;
        virtual void EndRender() = 0;
        /** @todo Model matrix should be optional, the only required arguments should be pipeline and mesh... and I am not sure about pipeline. */
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
        DLL void Render (
            const Transform& aModelTransform,
            const Mesh& aMesh,
            const Pipeline& aPipeline,
            const Material* aMaterial = nullptr,
            const BufferAccessor* aSkeleton = nullptr,
            uint32_t aVertexStart = 0,
            uint32_t aVertexCount = 0xffffffff,
            uint32_t aInstanceCount = 1,
            uint32_t aFirstInstance = 0 ) const;
        /** Render Scene in Full
         * @param aScene Scene to render
         * @note must be called between calls to BeginRender and EndRender
        */
        DLL void Render ( const Scene& aScene ) const;
        ///@}
        ///@name Matrix Functions
        ///@{
        DLL void SetProjectionMatrix ( const Matrix4x4& aMatrix );
        DLL void SetViewMatrix ( const Matrix4x4& aMatrix );
        DLL const Matrix4x4& GetProjectionMatrix() const;
        DLL const Matrix4x4& GetViewMatrix() const;
        ///@}
        DLL float GetAspectRatio() const;
        DLL const Frustum& GetFrustum() const;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual BufferAccessor AllocateSingleFrameUniformMemory ( size_t aSize ) = 0;
        virtual void WriteOverlayPixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Texture::Format aFormat, Texture::Type aType, const uint8_t* aPixels ) = 0;
    protected:
        virtual void OnResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) = 0;
        void* mWindowId{};
        Matrix4x4 mProjectionMatrix{};
        Matrix4x4 mViewMatrix{};
    private:
        virtual void OnSetProjectionMatrix() = 0;
        virtual void OnSetViewMatrix() = 0;
        Frustum mFrustum{};
        float mAspectRatio{1.0f};
    };
}
#endif

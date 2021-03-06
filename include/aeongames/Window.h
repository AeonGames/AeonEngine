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
#include <unordered_map>
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
        DLL virtual ~Window() = 0;
        virtual void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) = 0;
        virtual void Run ( Scene& aScene ) = 0;
        /** Set Scene for rendering.
         * @param aScene Pointer to the scene to be rendering
         * @note The aScene argument may be nullptr, in which case no rendering takes place.
         */
        virtual void SetScene ( const Scene* aScene ) = 0;
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
        virtual void Render (
            const Transform& aModelTransform,
            const Mesh& aMesh,
            const Pipeline& aPipeline,
            const Material* aMaterial = nullptr,
            const BufferAccessor* aSkeleton = nullptr,
            uint32_t aVertexStart = 0,
            uint32_t aVertexCount = 0xffffffff,
            uint32_t aInstanceCount = 1,
            uint32_t aFirstInstance = 0 ) const = 0;
        /** Render Scene in Full
         * @param aScene Scene to render
         * @note must be called between calls to BeginRender and EndRender
        */
        virtual void Render ( const Scene& aScene ) const = 0;
        /** Run a single render loop of the previously set scene.*/
        virtual void RenderLoop () = 0;
        ///@}
        ///@name Matrix Functions
        ///@{
        virtual void SetProjectionMatrix ( const Matrix4x4& aMatrix ) = 0;
        virtual void SetViewMatrix ( const Matrix4x4& aMatrix ) = 0;
        virtual const Matrix4x4& GetProjectionMatrix() const = 0;
        virtual const Matrix4x4& GetViewMatrix() const = 0;
        ///@}
        virtual float GetAspectRatio() const = 0;
        virtual const Frustum& GetFrustum() const = 0;
        virtual void Show ( bool aShow ) const = 0;
        virtual void StartRenderTimer() const = 0;
        virtual void StopRenderTimer() const = 0;
        DLL static Window* GetWindowFromId ( void* aId );
        virtual BufferAccessor AllocateSingleFrameUniformMemory ( size_t aSize ) = 0;
        virtual void WriteOverlayPixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Texture::Format aFormat, Texture::Type aType, const uint8_t* aPixels ) = 0;
    protected:
        DLL static void SetWindowForId ( void* aId, Window* aWindow );
        DLL static void RemoveWindowForId ( void* aId );
    private:
        static std::unordered_map<void*, Window*> WindowMap;
    };
}
#endif

/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_RENDERER_H
#define AEONGAMES_RENDERER_H

#include <string>
#include <memory>
#include <functional>
#include "Platform.h"
#include "aeongames/Matrix4x4.h"

namespace AeonGames
{
    class Frustum;
    class StringId;
    class Buffer;
    class Texture;
    class Mesh;
    class Pipeline;
    class Material;
    class Window;
    class BufferAccessor;
    class Renderer
    {
    public:
        DLL virtual ~Renderer() = 0;
        ///@name Renderer specific resource functions
        ///@{
        virtual void LoadMesh ( const Mesh& aMesh ) = 0;
        virtual void UnloadMesh ( const Mesh& aMesh ) = 0;
        virtual void LoadPipeline ( const Pipeline& aPipeline ) = 0;
        virtual void UnloadPipeline ( const Pipeline& aPipeline ) = 0;
        virtual void LoadMaterial ( const Material& aMaterial ) = 0;
        virtual void UnloadMaterial ( const Material& aMaterial ) = 0;
        virtual void LoadTexture ( const Texture& aTexture ) = 0;
        virtual void UnloadTexture ( const Texture& aTexture ) = 0;

        virtual void SetSkeleton ( const BufferAccessor& aSkeletonBuffer ) const = 0;
        virtual void BindMesh ( const Mesh& aMesh ) = 0;
        virtual void BindPipeline ( const Pipeline& aPipeline ) = 0;
        virtual void SetMaterial ( const Material& aMaterial ) = 0;
        virtual void SetModelMatrix ( const Matrix4x4& aMatrix ) = 0;
        virtual void SetProjectionMatrix ( const Matrix4x4& aMatrix ) = 0;
        virtual void SetViewMatrix ( const Matrix4x4& aMatrix ) = 0;
        ///@}

        ///@name Window surface related functions
        ///@{
        /**
         * Attach a Window as a rendering surface.
         * @param aWindowId Platform depended window handle.
        */
        virtual void AttachWindow ( void* aWindowId ) = 0;
        /**
         * Detach a Window as a rendering surface.
         * @param aWindowId Platform depended window handle.
        */
        virtual void DetachWindow ( void* aWindowId ) = 0;
        /** Sets the projection matrix matrix for a specific window surface.
         * @param aWindowId Platform depended window handle.
        */
        virtual void SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) = 0;
        /** Sets the view matrix matrix for a specific window surface.
         * @param aWindowId Platform depended window handle.
        */
        virtual void SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) = 0;
        /** Resizes the specific window surface's viewport.
         * @param aWindowId Platform depended window handle.
         * @param aX X coordinate of the viewport.
         * @param aY Y coordinate of the viewport.
         * @param aWidth Width of the viewport.
         * @param aHeight Height of the viewport.
         */
        virtual void ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) = 0;
        virtual void BeginRender ( void* aWindowId ) = 0;
        virtual void EndRender ( void* aWindowId ) = 0;
        /** @todo Model matrix should be optional, the only required arguments should be pipeline and mesh... and I am not sure about pipeline. */
        virtual void Render ( void* aWindowId,
                              const Matrix4x4& aModelMatrix,
                              const Mesh& aMesh,
                              const Pipeline& aPipeline,
                              const Material* aMaterial = nullptr,
                              const BufferAccessor* aSkeleton = nullptr,
                              uint32_t aVertexStart = 0,
                              uint32_t aVertexCount = 0xffffffff,
                              uint32_t aInstanceCount = 1,
                              uint32_t aFirstInstance = 0 ) const = 0;
        virtual const Frustum& GetFrustum ( void* aWindowId ) const = 0;
        virtual BufferAccessor AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize ) = 0;
        ///@}
    };
    /**@name Factory Functions */
    /*@{*/
    DLL std::unique_ptr<Renderer> ConstructRenderer ( uint32_t aIdentifier, void* aWindow );
    DLL std::unique_ptr<Renderer> ConstructRenderer ( const std::string& aIdentifier, void* aWindow );
    DLL std::unique_ptr<Renderer> ConstructRenderer ( const StringId& aIdentifier, void* aWindow );
    /** Registers a Renderer loader for a specific identifier.*/
    DLL bool RegisterRendererConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<Renderer> ( void* ) >& aConstructor );
    /** Unregisters a Renderer loader for a specific identifier.*/
    DLL bool UnregisterRendererConstructor ( const StringId& aIdentifier );
    /** Enumerates Renderer loader identifiers via an enumerator functor.*/
    DLL void EnumerateRendererConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /** Enumerates Renderer loader identifiers via an enumerator functor.*/
    DLL std::vector<std::string> GetRendererConstructorNames();
    /*@}*/
}
#endif

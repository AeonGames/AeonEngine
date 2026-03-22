/*
Copyright (C) 2016-2022,2024-2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Platform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Pipeline.hpp"

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
    /** Abstract base class for rendering backends.
     *
     * Defines the interface for loading and unloading GPU resources, managing
     * window surfaces, and issuing draw calls. Concrete implementations provide
     * API-specific logic (e.g., Vulkan, OpenGL).
     */
    class Renderer
    {
    public:
        /** Virtual destructor. */
        DLL virtual ~Renderer() = 0;
        ///@name Renderer specific resource functions
        ///@{
        /** Loads mesh data into GPU memory.
         * @param aMesh The mesh to load.
         */
        virtual void LoadMesh ( const Mesh& aMesh ) = 0;
        /** Unloads mesh data from GPU memory.
         * @param aMesh The mesh to unload.
         */
        virtual void UnloadMesh ( const Mesh& aMesh ) = 0;
        /** Loads a rendering pipeline (shaders and state) into the renderer.
         * @param aPipeline The pipeline to load.
         */
        virtual void LoadPipeline ( const Pipeline& aPipeline ) = 0;
        /** Unloads a rendering pipeline from the renderer.
         * @param aPipeline The pipeline to unload.
         */
        virtual void UnloadPipeline ( const Pipeline& aPipeline ) = 0;
        /** Loads material data into the renderer.
         * @param aMaterial The material to load.
         */
        virtual void LoadMaterial ( const Material& aMaterial ) = 0;
        /** Unloads material data from the renderer.
         * @param aMaterial The material to unload.
         */
        virtual void UnloadMaterial ( const Material& aMaterial ) = 0;
        /** Loads a texture into GPU memory.
         * @param aTexture The texture to load.
         */
        virtual void LoadTexture ( const Texture& aTexture ) = 0;
        /** Unloads a texture from GPU memory.
         * @param aTexture The texture to unload.
         */
        virtual void UnloadTexture ( const Texture& aTexture ) = 0;
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
        /** Sets the projection matrix for a specific window surface.
         * @param aWindowId Platform depended window handle.
         * @param aMatrix The projection matrix.
        */
        virtual void SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) = 0;
        /** Sets the view matrix for a specific window surface.
         * @param aWindowId Platform depended window handle.
         * @param aMatrix The view matrix.
        */
        virtual void SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) = 0;
        /** Sets the color to be used to clear the window background.
         * @param aWindowId Platform depended window handle.
         * @param R Red component of the clear color.
         * @param G Green component of the clear color.
         * @param B Blue component of the clear color.
         * @param A Alpha component of the clear color.
        */
        virtual void SetClearColor ( void* aWindowId, float R, float G, float B, float A ) = 0;
        /** Resizes the specific window surface's viewport.
         * @param aWindowId Platform dependent window handle.
         * @param aX X coordinate of the viewport.
         * @param aY Y coordinate of the viewport.
         * @param aWidth Width of the viewport.
         * @param aHeight Height of the viewport.
         */
        virtual void ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) = 0;
        /** Begins a render pass for the given window surface.
         * @param aWindowId Platform dependent window handle.
         */
        virtual void BeginRender ( void* aWindowId ) = 0;
        /** Ends the current render pass for the given window surface.
         * @param aWindowId Platform dependent window handle.
         */
        virtual void EndRender ( void* aWindowId ) = 0;
        /** Issues a draw call for a mesh with the given pipeline and optional material.
         * @param aWindowId Platform dependent window handle.
         * @param aModelMatrix Model transformation matrix.
         * @param aMesh Mesh to render.
         * @param aPipeline Pipeline (shaders/state) to use.
         * @param aMaterial Optional material to bind.
         * @param aSkeleton Optional skeleton buffer for skinned meshes.
         * @param aTopology Primitive topology (default: TRIANGLE_LIST).
         * @param aVertexStart First vertex index.
         * @param aVertexCount Number of vertices to draw (default: all).
         * @param aInstanceCount Number of instances to draw.
         * @param aFirstInstance Index of the first instance.
         */
        virtual void Render ( void* aWindowId,
                              const Matrix4x4& aModelMatrix,
                              const Mesh& aMesh,
                              const Pipeline& aPipeline,
                              const Material* aMaterial = nullptr,
                              const BufferAccessor* aSkeleton = nullptr,
                              Topology aTopology = Topology::TRIANGLE_LIST,
                              uint32_t aVertexStart = 0,
                              uint32_t aVertexCount = 0xffffffff,
                              uint32_t aInstanceCount = 1,
                              uint32_t aFirstInstance = 0 ) const = 0;
        /** Returns the view frustum for the given window surface.
         * @param aWindowId Platform dependent window handle.
         * @return A const reference to the current view frustum.
         */
        virtual const Frustum& GetFrustum ( void* aWindowId ) const = 0;
        /** Allocates uniform buffer memory that is valid for a single frame.
         * @param aWindowId Platform dependent window handle.
         * @param aSize Size in bytes of the requested allocation.
         * @return A BufferAccessor to the allocated memory.
         */
        virtual BufferAccessor AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize ) = 0;
        ///@}
    };
    /**@name Factory Functions */
    /*@{*/
    /** Constructs a Renderer identified by a numeric identifier.
     * @param aIdentifier Numeric renderer identifier.
     * @param aWindow Platform dependent window handle.
     * @return A unique_ptr to the newly created Renderer.
     */
    DLL std::unique_ptr<Renderer> ConstructRenderer ( uint32_t aIdentifier, void* aWindow );
    /** Constructs a Renderer identified by a string name.
     * @param aIdentifier String renderer identifier.
     * @param aWindow Platform dependent window handle.
     * @return A unique_ptr to the newly created Renderer.
     */
    DLL std::unique_ptr<Renderer> ConstructRenderer ( const std::string& aIdentifier, void* aWindow );
    /** Constructs a Renderer identified by a StringId.
     * @param aIdentifier StringId renderer identifier.
     * @param aWindow Platform dependent window handle.
     * @return A unique_ptr to the newly created Renderer.
     */
    DLL std::unique_ptr<Renderer> ConstructRenderer ( const StringId& aIdentifier, void* aWindow );
    /** Registers a Renderer loader for a specific identifier.*/
    DLL bool RegisterRendererConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<Renderer> ( void* ) >& aConstructor );
    /** Unregisters a Renderer loader for a specific identifier.*/
    DLL bool UnregisterRendererConstructor ( const StringId& aIdentifier );
    /** Enumerates Renderer loader identifiers via an enumerator functor.*/
    DLL void EnumerateRendererConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /** Returns the names of all registered Renderer constructors.
     * @return A vector of registered renderer constructor name strings.
     */
    DLL std::vector<std::string> GetRendererConstructorNames();
    /*@}*/
}
#endif

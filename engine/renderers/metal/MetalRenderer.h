/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_METALRENDERER_H
#define AEONGAMES_METALRENDERER_H

#include <memory>
#include "aeongames/Renderer.hpp"

namespace AeonGames
{
    /** @brief Native Metal 3 rendering backend for Apple Silicon. */
    class MetalRenderer final : public Renderer
    {
    public:
        MetalRenderer ( void* aWindow, const RendererSettings& aSettings );
        ~MetalRenderer() final;
        MetalRenderer ( const MetalRenderer& ) = delete;
        MetalRenderer& operator= ( const MetalRenderer& ) = delete;

        std::string_view GetName() const final;
        const RendererSettings& GetSettings() const final;

        void LoadMesh ( const Mesh& aMesh ) final;
        void UnloadMesh ( const Mesh& aMesh ) final;
        void LoadPipeline ( const Pipeline& aPipeline ) final;
        void UnloadPipeline ( const Pipeline& aPipeline ) final;
        void LoadMaterial ( const Material& aMaterial ) final;
        void UnloadMaterial ( const Material& aMaterial ) final;
        void LoadTexture ( const Texture& aTexture ) final;
        void UnloadTexture ( const Texture& aTexture ) final;

        void AttachWindow ( void* aWindowId ) final;
        void DetachWindow ( void* aWindowId ) final;
        void SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void SetLights ( void* aWindowId, std::span<const GpuLight> aLights ) final;
        void SetGlobals ( void* aWindowId, const GpuGlobals& aGlobals ) final;
        void SetEnvironmentMap ( void* aWindowId, const Texture* aEnvironmentMap ) final;
        void SetClearColor ( void* aWindowId, float aRed, float aGreen, float aBlue, float aAlpha ) final;
        void ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) final;
        void BeginRender ( void* aWindowId, const Pipeline* aComputePipeline = nullptr ) final;
        void BeginFrame ( void* aWindowId ) final;
        void BeginRenderPass ( void* aWindowId ) final;
        void BeginShadowPass ( void* aWindowId, const Matrix4x4& aLightViewProjection ) final;
        void EndShadowPass ( void* aWindowId ) final;
        void SetSpotShadowParams ( void* aWindowId, const GpuSpotShadowParams& aSpotShadowParams ) final;
        void BeginSpotShadowPass ( void* aWindowId, uint32_t aSlot, const Matrix4x4& aLightViewProjection ) final;
        void EndSpotShadowPass ( void* aWindowId ) final;
        void SetPointShadowParams ( void* aWindowId, const GpuPointShadowParams& aPointShadowParams ) final;
        void BeginPointShadowPass ( void* aWindowId, uint32_t aCaster ) final;
        void EndPointShadowPass ( void* aWindowId ) final;
        void EndDepthPrePass ( void* aWindowId, const Pipeline* aComputePipeline ) final;
        void EndRender ( void* aWindowId ) final;
        void Finish ( void* aWindowId ) final;
        void RequestCapture ( void* aWindowId ) final;
        bool ReadPixels ( void* aWindowId, Texture& aTexture ) const final;

        void Render ( void* aWindowId,
                      const Matrix4x4& aModelMatrix,
                      const Mesh& aMesh,
                      const Pipeline& aPipeline,
                      const Material* aMaterial = nullptr,
                      Topology aTopology = Topology::TRIANGLE_LIST,
                      uint32_t aVertexStart = 0,
                      uint32_t aVertexCount = 0xffffffff,
                      uint32_t aInstanceCount = 1,
                      uint32_t aFirstInstance = 0,
                      const BufferAccessor* aSkinnedVertices = nullptr,
                      RenderPass aRenderPass = RenderPass::Shading ) const final;
        void RenderInstanced ( void* aWindowId,
                               std::span<const Matrix4x4> aModelMatrices,
                               const Mesh& aMesh,
                               const Pipeline& aPipeline,
                               const Material* aMaterial = nullptr,
                               Topology aTopology = Topology::TRIANGLE_LIST,
                               uint32_t aVertexStart = 0,
                               uint32_t aVertexCount = 0xffffffff,
                               RenderPass aRenderPass = RenderPass::Shading ) final;
        void Dispatch ( void* aWindowId,
                        const Pipeline& aPipeline,
                        uint32_t aGroupCountX,
                        uint32_t aGroupCountY = 1,
                        uint32_t aGroupCountZ = 1,
                        std::span<const StorageBufferBinding> aStorageBuffers = {},
                        uint32_t aComputeStageIndex = 0 ) const final;
        void Skin ( void* aWindowId,
                    const Pipeline& aSkinningPipeline,
                    const Mesh& aMesh,
                    const BufferAccessor& aSkinningMatrices,
                    const BufferAccessor& aSkinnedVertices ) const final;
        void Barrier ( void* aWindowId ) const final;
        const Frustum& GetFrustum ( void* aWindowId ) const final;
        const Matrix4x4& GetProjectionMatrix ( void* aWindowId ) const final;
        const BufferAccessor* GetFrameLightGrid ( void* aWindowId ) const final;
        const BufferAccessor* GetFrameClusterActive ( void* aWindowId ) const final;
        BufferAccessor AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize ) final;
        BufferAccessor AllocateSingleFrameStorageMemory ( void* aWindowId, size_t aSize ) final;
        void RenderOverlay ( void* aWindowId, const GuiOverlay& aGuiOverlay ) final;

    protected:
        void SubmitRenderQueue ( void* aWindowId, const Scene& aScene, RenderPass aRenderPass ) final;
        bool IsValidWindow ( void* aWindowId ) const final;

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
#endif
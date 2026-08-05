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
#ifndef AEONGAMES_METALWINDOW_H
#define AEONGAMES_METALWINDOW_H

#include <memory>
#include "aeongames/Renderer.hpp"

namespace AeonGames
{
    class MetalArgumentBufferPool;
    /** @brief Per-window Metal frame and attachment state. */
    class MetalWindow
    {
    public:
        MetalWindow ( void* aDevice, void* aCommandQueue, const RendererSettings& aSettings, void* aWindowId );
        ~MetalWindow();

        void SetProjectionMatrix ( const Matrix4x4& aMatrix );
        void SetViewMatrix ( const Matrix4x4& aMatrix );
        void SetLights ( std::span<const GpuLight> aLights );
        void SetGlobals ( const GpuGlobals& aGlobals );
        void SetSpotShadowParams ( const GpuSpotShadowParams& aSpotShadowParams );
        void SetPointShadowParams ( const GpuPointShadowParams& aPointShadowParams );
        void SetEnvironmentMap ( const Texture* aEnvironmentMap );
        void SetClearColor ( float aRed, float aGreen, float aBlue, float aAlpha );
        void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight );
        void BeginFrame();
        void PrepareClusters();
        void BeginRenderPass();
        void BeginShadowPass ( const Matrix4x4& aLightViewProjection );
        void EndShadowPass();
        void BeginSpotShadowPass ( uint32_t aSlot, const Matrix4x4& aLightViewProjection );
        void EndSpotShadowPass();
        void BeginPointShadowPass ( uint32_t aCaster );
        void EndPointShadowPass();
        void EndDepthPrePass();
        void BuildHiZ ( MetalPipeline& aPipeline );
        void ResolveFrame ( MetalPipeline& aSkyboxPipeline, MetalPipeline& aTonemapPipeline );
        void RenderOverlay ( void* aPipeline, void* aSampler, void* aDepthState, const uint8_t* aPixels,
                             uint32_t aWidth, uint32_t aHeight );
        void EndRender();
        void Finish() const;
        void RequestCapture();
        bool ReadPixels ( Texture& aTexture ) const;
        const Frustum& GetFrustum() const;
        const Matrix4x4& GetProjectionMatrix() const;
        void* GetCommandBuffer();
        void* GetRenderEncoder();
        /** Frame-local scratch for the argument buffers a draw or dispatch binds. */
        MetalArgumentBufferPool& GetArgumentBufferPool();
        BufferAccessor AllocateUniform ( size_t aSize );
        BufferAccessor AllocateStorage ( size_t aSize );
        const BufferAccessor& GetClusterParams();
        const BufferAccessor& GetMatrices();
        const BufferAccessor& GetLights();
        const BufferAccessor& GetGlobals();
        const BufferAccessor& GetShadowParams();
        const BufferAccessor& GetSpotShadowParams();
        const BufferAccessor& GetPointShadowParams();
        const BufferAccessor& GetShadowDepthParams();
        const BufferAccessor& GetPointShadowDepthParams();
        const BufferAccessor& GetClusterAABBs() const;
        const BufferAccessor& GetLightGrid() const;
        const BufferAccessor& GetLightIndexList() const;
        const BufferAccessor& GetLightIndexCounter() const;
        const BufferAccessor& GetClusterActive() const;
        const BufferAccessor* GetFrameLightGrid() const;
        const BufferAccessor* GetFrameClusterActive() const;
        void* GetShadowMap() const;
        void* GetSpotShadowMap() const;
        void* GetPointShadowMap() const;
        void* GetShadowSampler() const;
        void* GetHiZTexture() const;
        void* GetLinearSampler() const;
        void* GetHiZSampler() const;
        bool HasHiZ() const;
        bool IsPointShadowPass() const;

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
#endif
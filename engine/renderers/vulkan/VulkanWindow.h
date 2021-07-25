/*
Copyright (C) 2017-2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANWINDOW_H
#define AEONGAMES_VULKANWINDOW_H

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "aeongames/Matrix4x4.h"
#include "aeongames/Frustum.h"
#include "VulkanMemoryPoolBuffer.h"

namespace AeonGames
{
    class Mesh;
    class Material;
    class Pipeline;
    class BufferAccessor;
    class VulkanRenderer;
    class VulkanWindow
    {
    public:
        VulkanWindow ( VulkanRenderer& aVulkanRenderer, void* aWindowId );
        ~VulkanWindow();
        VulkanWindow ( VulkanWindow&& aVulkanWindow );
        VulkanWindow ( const VulkanWindow& aVulkanWindow ) = delete;
        VulkanWindow& operator= ( const VulkanWindow& aVulkanWindow ) = delete;
        VulkanWindow& operator= ( VulkanWindow&& aVulkanWindow ) = delete;

        void BeginRender();
        void EndRender();
        void Render (   const Matrix4x4& aModelMatrix,
                        const Mesh& aMesh,
                        const Pipeline& aPipeline,
                        const Material* aMaterial = nullptr,
                        const BufferAccessor* aSkeleton = nullptr,
                        uint32_t aVertexStart = 0,
                        uint32_t aVertexCount = 0xffffffff,
                        uint32_t aInstanceCount = 1,
                        uint32_t aFirstInstance = 0 ) const;
        void SetProjectionMatrix ( const Matrix4x4& aMatrix );
        void SetViewMatrix ( const Matrix4x4& aMatrix );
        const Matrix4x4& GetProjectionMatrix() const;
        const Matrix4x4& GetViewMatrix() const;
        const Frustum& GetFrustum() const;
        void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight );
        BufferAccessor AllocateSingleFrameUniformMemory ( size_t aSize );
    private:
        void Initialize();
        void Finalize();
        void InitializeSurface();
        void InitializeSwapchain();
        void InitializeImageViews();
        void InitializeDepthStencil();
        void InitializeFrameBuffers();
        void FinalizeSurface();
        void FinalizeSwapchain();
        void FinalizeImageViews();
        void FinalizeDepthStencil();
        void FinalizeFrameBuffers();
        VulkanRenderer& mVulkanRenderer;
        void* mWindowId{};
        Frustum mFrustum{};
#if defined(__unix__)
        Colormap mColorMap {};
#elif defined(_WIN32)
        HDC mDeviceContext {};
#endif
        VulkanMemoryPoolBuffer mMemoryPoolBuffer;
        Matrix4x4 mProjectionMatrix {};
        Matrix4x4 mViewMatrix{};
        VkSurfaceKHR mVkSurfaceKHR{ VK_NULL_HANDLE };
        VkSurfaceCapabilitiesKHR mVkSurfaceCapabilitiesKHR {};
        uint32_t mSwapchainImageCount{ 2 };
        VkSwapchainKHR mVkSwapchainKHR{ VK_NULL_HANDLE };
        VkImage mVkDepthStencilImage{ VK_NULL_HANDLE };
        VkDeviceMemory mVkDepthStencilImageMemory{ VK_NULL_HANDLE };
        VkImageView mVkDepthStencilImageView { VK_NULL_HANDLE};
        bool mHasStencil{ false };
        uint32_t mActiveImageIndex{ UINT32_MAX };
        VkViewport mVkViewport{0, 0, 1, 1, 0, 1};
        VkRect2D mVkScissor{};
        ::std::vector<VkImage> mVkSwapchainImages{};
        ::std::vector<VkImageView> mVkSwapchainImageViews{};
        ::std::vector<VkFramebuffer> mVkFramebuffers{};
    };
}
#endif

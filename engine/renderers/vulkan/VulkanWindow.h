/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Window.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanWindow : public Window
    {
    public:
        VulkanWindow ( void* aWindowId, const VulkanRenderer&  aVulkanRenderer );
        virtual ~VulkanWindow();
        void ResizeViewport ( uint32_t aWidth, uint32_t aHeight ) final;
        void BeginRender() const final;
        void EndRender() const final;
        void Render (   const Transform& aModelTransform,
                        const Mesh& aMesh,
                        const Pipeline& aPipeline,
                        const Material* aMaterial = nullptr,
                        uint32_t aVertexStart = 0,
                        uint32_t aVertexCount = 0xffffffff,
                        uint32_t aInstanceCount = 1,
                        uint32_t aFirstInstance = 0 ) const final;
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
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        void InitializeDescriptorSet();
        void FinalizeDescriptorSet();
        void* mWindowId{};
        VkSurfaceKHR mVkSurfaceKHR{ VK_NULL_HANDLE };
        const VulkanRenderer& mVulkanRenderer;
        VulkanBuffer mMatrices;
        VkSurfaceCapabilitiesKHR mVkSurfaceCapabilitiesKHR {};
        uint32_t mSwapchainImageCount{ 2 };
        VkSwapchainKHR mVkSwapchainKHR{ VK_NULL_HANDLE };
        ::std::vector<VkImage> mVkSwapchainImages{};
        ::std::vector<VkImageView> mVkSwapchainImageViews{};
        VkImage mVkDepthStencilImage{ VK_NULL_HANDLE };
        VkDeviceMemory mVkDepthStencilImageMemory{ VK_NULL_HANDLE };
        VkImageView mVkDepthStencilImageView { VK_NULL_HANDLE};
        bool mHasStencil{ false };
        ::std::vector<VkFramebuffer> mVkFramebuffers{};
        uint32_t mActiveImageIndex{ UINT32_MAX };
        VkViewport mVkViewport{};
        VkRect2D mVkScissor{};
        VkDescriptorPool mVkMatricesDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSet mVkMatricesDescriptorSet{ VK_NULL_HANDLE };
    };
}
#endif

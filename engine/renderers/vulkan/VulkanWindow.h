/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanWindow
    {
    public:
        VulkanWindow ( uintptr_t aWindowId, const VulkanRenderer* aVulkanRenderer );
        ~VulkanWindow();
        uintptr_t GetWindowId() const;
        void BeginRender() const;
        void EndRender() const;
    private:
        void Initialize();
        void Finalize();
        void CreateSurface();
        void CreateSwapchain();
        uintptr_t mWindowId;
        VkSurfaceKHR mVkSurfaceKHR = VK_NULL_HANDLE;
        const VulkanRenderer* mVulkanRenderer;
        VkSurfaceCapabilitiesKHR mVkSurfaceCapabilitiesKHR {};
        VkSurfaceFormatKHR mVkSurfaceFormatKHR{};
        uint32_t mSwapchainImageCount = 2;
        VkSwapchainKHR mVkSwapchainKHR = VK_NULL_HANDLE;
        VkViewport mVkViewport = {};
        std::vector<VkImage> mVkSwapchainImages;
        std::vector<VkImageView> mVkSwapchainImageViews;
        VkImage mVkDepthStencilImage = VK_NULL_HANDLE;
        VkDeviceMemory mVkDepthStencilImageMemory = VK_NULL_HANDLE;
        VkImageView mVkDepthStencilImageView = VK_NULL_HANDLE;
        VkFormat mVkDepthStencilFormat = VK_FORMAT_UNDEFINED;
        bool mHasStencil = false;
        VkRenderPass mVkRenderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> mVkFramebuffers;
        uint32_t mActiveImageIndex = UINT32_MAX;
        VkFence mVkFence = VK_NULL_HANDLE;
        /* Not sure if the following should be here */
        VkCommandPool mVkCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer mVkCommandBuffer = VK_NULL_HANDLE;
    };
}
#endif

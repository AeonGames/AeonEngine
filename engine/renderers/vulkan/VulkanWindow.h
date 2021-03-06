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
#include "VulkanMemoryPoolBuffer.h"
#include "aeongames/CommonWindow.h"
#include "aeongames/MemoryPool.h" ///<- This is here just for the literals
#ifdef __unix__
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanWindow final : public CommonWindow
    {
    public:
        VulkanWindow ( VulkanRenderer& aVulkanRenderer, void* aWindowId );
        VulkanWindow ( VulkanRenderer& aVulkanRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        ~VulkanWindow() final;
        void BeginRender() final;
        void EndRender() final;
        void Render (   const Matrix4x4& aModelMatrix,
                        const Mesh& aMesh,
                        const Pipeline& aPipeline,
                        const Material* aMaterial = nullptr,
                        const BufferAccessor* aSkeleton = nullptr,
                        uint32_t aVertexStart = 0,
                        uint32_t aVertexCount = 0xffffffff,
                        uint32_t aInstanceCount = 1,
                        uint32_t aFirstInstance = 0 ) const final;
        BufferAccessor AllocateSingleFrameUniformMemory ( size_t aSize ) final;
        void WriteOverlayPixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Texture::Format aFormat, Texture::Type aType, const uint8_t* aPixels ) final;
        void SetProjectionMatrix ( const Matrix4x4& aMatrix ) final;
        void SetViewMatrix ( const Matrix4x4& aMatrix ) final;
        const Matrix4x4 & GetProjectionMatrix() const final;
        const Matrix4x4 & GetViewMatrix() const final;
        void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) final;

        void Run ( Scene& aScene ) final;
        void Show ( bool aShow ) const final;
        void StartRenderTimer() const final;
        void StopRenderTimer() const final;
#ifdef __unix__
        static GLXFBConfig GetGLXConfig ( Display* display );
#endif
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
#if defined(__unix__)
        Colormap mColorMap {};
#elif defined(_WIN32)
        HDC mDeviceContext {};
#endif
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
        VkViewport mVkViewport{0, 0, 0, 0, 0, 1};
        VkRect2D mVkScissor{};
        ::std::vector<VkImage> mVkSwapchainImages{};
        ::std::vector<VkImageView> mVkSwapchainImageViews{};
        ::std::vector<VkFramebuffer> mVkFramebuffers{};
    };
}
#endif

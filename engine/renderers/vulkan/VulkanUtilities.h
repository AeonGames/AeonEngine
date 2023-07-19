/*
Copyright (C) 2017-2019,2021,2023 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANUTILITIES_H
#define AEONGAMES_VULKANUTILITIES_H
#include "vulkan/vulkan.h"
#ifdef Status
#undef Status
#endif
#include "aeongames/Pipeline.h"
namespace AeonGames
{
    static_assert ( VK_SUCCESS == 0, "VK_SUCCESS is NOT zero!" );
    const char* GetVulkanResultString ( VkResult aResult );
    VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback
    (
        VkDebugUtilsMessageSeverityFlagBitsEXT           aMessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  aMessageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT*      aCallbackData,
        void*                                            aUserData
    );

    VkDescriptorPool CreateDescriptorPool ( const VkDevice& aVkDevice, const std::vector<VkDescriptorPoolSize>& aVkDescriptorPoolSizes );
    void DestroyDescriptorPool ( const VkDevice& aVkDevice, VkDescriptorPool aVkDescriptorPool );
    VkDescriptorSet CreateDescriptorSet ( const VkDevice& aVkDevice, const VkDescriptorPool& aVkDescriptorPool, const VkDescriptorSetLayout& aVkDescriptorSetLayout, uint32_t aDescriptorSetCount = 1 );
}
#endif

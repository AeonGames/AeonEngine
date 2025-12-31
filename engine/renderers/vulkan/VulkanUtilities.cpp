/*
Copyright (C) 2017-2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include "VulkanUtilities.h"
#include "aeongames/LogLevel.hpp"
#include <iostream>
#include <sstream>
namespace AeonGames
{
    const char* GetVulkanResultString ( VkResult aResult )
    {
        switch ( aResult )
        {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:
            return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
            return "VK_ERROR_OUT_OF_POOL_MEMORY_KHR";
        default:
            return "Unknown Result";
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback
    (
        VkDebugUtilsMessageSeverityFlagBitsEXT           aMessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  aMessageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT*      aCallbackData,
        void*                                            aUserData
    )
    {

        if ( aMessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
        {
            std::cout << LogLevel::Info;
        }
        if ( aMessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
        {
            std::cout << LogLevel::Warning;
        }
#if 0
        if ( aMessageSeverity & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT )
        {
            std::cout << LogLevel::Performance;
        }
#endif
        if ( aMessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
        {
            std::cout << LogLevel::Error;
        }
        if ( aMessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT )
        {
            std::cout << LogLevel::Debug;
        }
        std::cout << aCallbackData->pMessageIdName << ": " << aCallbackData->pMessage << std::endl;
        return false;
    }

    VkDescriptorPool CreateDescriptorPool ( const VkDevice& aVkDevice, const std::vector<VkDescriptorPoolSize>& aVkDescriptorPoolSizes )
    {
        VkDescriptorPool descriptor_pool{VK_NULL_HANDLE};
        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.pNext = nullptr;
        descriptor_pool_create_info.flags = 0;
        descriptor_pool_create_info.maxSets = static_cast<uint32_t> ( aVkDescriptorPoolSizes.size() );
        descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t> ( aVkDescriptorPoolSizes.size() );
        descriptor_pool_create_info.pPoolSizes = aVkDescriptorPoolSizes.data();
        if ( VkResult result = vkCreateDescriptorPool ( aVkDevice, &descriptor_pool_create_info, nullptr, &descriptor_pool ) )
        {
            std::ostringstream stream;
            stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        return descriptor_pool;
    }

    void DestroyDescriptorPool ( const VkDevice& aVkDevice, VkDescriptorPool aVkDescriptorPool )
    {
        vkDestroyDescriptorPool ( aVkDevice, aVkDescriptorPool, nullptr );
    }

    VkDescriptorSet CreateDescriptorSet ( const VkDevice& aVkDevice, const VkDescriptorPool& aVkDescriptorPool, const VkDescriptorSetLayout& aVkDescriptorSetLayout, uint32_t aDescriptorSetCount )
    {
        VkDescriptorSet descriptor_set{VK_NULL_HANDLE};
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = aVkDescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount = aDescriptorSetCount;
        descriptor_set_allocate_info.pSetLayouts = &aVkDescriptorSetLayout;
        if ( VkResult result = vkAllocateDescriptorSets ( aVkDevice, &descriptor_set_allocate_info, &descriptor_set ) )
        {
            std::ostringstream stream;
            stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        return descriptor_set;
    }
}

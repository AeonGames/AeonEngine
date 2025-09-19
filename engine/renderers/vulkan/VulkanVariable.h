/*
Copyright (C) 2025 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANVARIABLE_H
#define AEONGAMES_VULKANVARIABLE_H

#include <cstdint>
#include <vulkan/vulkan.h>

namespace AeonGames
{
    struct VulkanVariable
    {
        uint32_t name;
        union
        {
            uint32_t binding;
            uint32_t location;
            uint32_t offset;
        };
        uint32_t size;
        VkFormat format;
        uint32_t descriptorType;
    };
}
#endif
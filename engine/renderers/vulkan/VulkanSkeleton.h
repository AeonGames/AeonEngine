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
#ifndef AEONGAMES_VULKANSKELETON_H
#define AEONGAMES_VULKANSKELETON_H
#include "aeongames/Memory.h"
#include <vulkan/vulkan.h>
#include "VulkanBuffer.h"
namespace AeonGames
{
    class Skeleton;
    class Animation;
    class VulkanRenderer;
    class VulkanSkeleton
    {
    public:
        VulkanSkeleton ( const std::shared_ptr<const Skeleton>&  aSkeleton, const std::shared_ptr<const VulkanRenderer>& aVulkanRenderer );
        ~VulkanSkeleton();
        VkBuffer GetBuffer() const;
        void SetPose ( const std::vector<Matrix4x4>& aSkeleton ) const;
        size_t GetBufferSize() const;
    private:
        void Initialize();
        void Finalize();
        std::shared_ptr<const VulkanRenderer> mVulkanRenderer;
        std::shared_ptr<const Skeleton> mSkeleton;
        VulkanBuffer mSkeletonBuffer;
    };
}
#endif

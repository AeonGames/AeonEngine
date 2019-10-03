/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANFRAMEBUFFER_H
#define AEONGAMES_VULKANFRAMEBUFFER_H
#include <cstddef>
#include "aeongames/FrameBuffer.h"

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanFrameBuffer : public FrameBuffer
    {
    public:
        VulkanFrameBuffer ( uint32_t aPath = 0 );
        ~VulkanFrameBuffer() final;
        void Load ( uint32_t aId ) final;
        void Load ( const std::string& aFilename ) final;
        void Load ( const void* aBuffer, size_t aBufferSize ) final;
        void Load ( const FrameBufferBuffer& aFrameBufferBuffer ) final;
        void Unload () final;
    private:
    };
}
#endif

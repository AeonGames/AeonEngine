/*
Copyright (C) 2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.hpp"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "framebuffer.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "VulkanFrameBuffer.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/AeonEngine.hpp"

namespace AeonGames
{
    VulkanFrameBuffer::VulkanFrameBuffer() = default;
    VulkanFrameBuffer::~VulkanFrameBuffer()
    {
        Unload();
    }
    void VulkanFrameBuffer::LoadFromPBMsg ( const FrameBufferMsg& aFrameBufferMsg )
    {
        ///@todo Write Me
    }
    void VulkanFrameBuffer::Unload ()
    {
        ///@todo Write Me
    }
    void VulkanFrameBuffer::Resize ( uint32_t aWidth, uint32_t aHeight ) {}
    void VulkanFrameBuffer::Bind() {}
    void VulkanFrameBuffer::Unbind() {}
}

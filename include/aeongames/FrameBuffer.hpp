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
#ifndef AEONGAMES_FRAMEBUFFER_H
#define AEONGAMES_FRAMEBUFFER_H
#include <cstdint>
#include <string>
#include "aeongames/Platform.hpp"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "framebuffer.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Resource.hpp"
namespace AeonGames
{
    class FrameBufferMsg;
    /** @brief Abstract interface for off-screen render target (frame buffer) resources. */
    class FrameBuffer : public Resource
    {
    public:
        DLL virtual ~FrameBuffer() = 0;
        /**
         * @brief Initialize the frame buffer from a protobuf message.
         * @param aFrameBufferMsg Protobuf message describing the frame buffer configuration.
         */
        virtual void LoadFromPBMsg ( const FrameBufferMsg& aFrameBufferMsg ) = 0;
        /** @brief Release all resources held by this frame buffer. */
        virtual void Unload() = 0;
        /**
         * @brief Resize the frame buffer to new dimensions.
         * @param aWidth  New width in pixels.
         * @param aHeight New height in pixels.
         */
        virtual void Resize ( uint32_t aWidth, uint32_t aHeight ) = 0;
        /** @brief Bind this frame buffer as the current render target. */
        virtual void Bind() = 0;
        /** @brief Unbind this frame buffer, restoring the previous render target. */
        virtual void Unbind() = 0;
    };
}
#endif

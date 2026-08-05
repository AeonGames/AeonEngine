/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_METALMEMORYPOOLBUFFER_H
#define AEONGAMES_METALMEMORYPOOLBUFFER_H

#include <memory>
#include "aeongames/MemoryPoolBuffer.hpp"

namespace AeonGames
{
    class MetalBuffer;
    /** @brief Linear, frame-local suballocator over one shared Metal buffer. */
    class MetalMemoryPoolBuffer final : public MemoryPoolBuffer
    {
    public:
        MetalMemoryPoolBuffer ( void* aDevice, size_t aCapacity, size_t aAlignment );
        ~MetalMemoryPoolBuffer() final;
        MetalMemoryPoolBuffer ( MetalMemoryPoolBuffer&& ) noexcept;
        MetalMemoryPoolBuffer& operator= ( MetalMemoryPoolBuffer&& ) noexcept;
        MetalMemoryPoolBuffer ( const MetalMemoryPoolBuffer& ) = delete;
        MetalMemoryPoolBuffer& operator= ( const MetalMemoryPoolBuffer& ) = delete;

        BufferAccessor Allocate ( size_t aSize ) final;
        void Reset() final;
        const Buffer& GetBuffer() const final;

    private:
        std::unique_ptr<MetalBuffer> mBuffer;
        size_t mOffset{0};
        size_t mAlignment{1};
    };
}
#endif
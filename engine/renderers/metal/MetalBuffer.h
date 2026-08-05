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
#ifndef AEONGAMES_METALBUFFER_H
#define AEONGAMES_METALBUFFER_H

#include <memory>
#include "aeongames/Buffer.hpp"

namespace AeonGames
{
    /** @brief CPU-visible Metal buffer used by transient frame allocations. */
    class MetalBuffer final : public Buffer
    {
    public:
        MetalBuffer ( void* aDevice, size_t aSize );
        ~MetalBuffer() final;
        MetalBuffer ( MetalBuffer&& ) noexcept;
        MetalBuffer& operator= ( MetalBuffer&& ) noexcept;
        MetalBuffer ( const MetalBuffer& ) = delete;
        MetalBuffer& operator= ( const MetalBuffer& ) = delete;

        void WriteMemory ( size_t aOffset, size_t aSize, const void* aData = nullptr ) const final;
        void* Map ( size_t aOffset, size_t aSize ) const final;
        void Unmap() const final;
        size_t GetSize() const final;
        /** @return Unretained bridge to the backing id<MTLBuffer>. */
        void* GetNativeBuffer() const;

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
#endif
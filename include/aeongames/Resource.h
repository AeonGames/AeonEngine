/*
Copyright (C) 2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_RESOURCE_H
#define AEONGAMES_RESOURCE_H

#include <cstdint>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <array>
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"
#include "aeongames/ProtobufHelpers.h"

namespace AeonGames
{
    constexpr uint64_t operator "" _mgk ( const char* literal, const std::size_t ) noexcept
    {
        /// @todo add support for Big Endian
        return
            ( static_cast<uint64_t> ( literal[7] ) << 56 ) |
            ( static_cast<uint64_t> ( literal[6] ) << 48 ) |
            ( static_cast<uint64_t> ( literal[5] ) << 40 ) |
            ( static_cast<uint64_t> ( literal[4] ) << 32 ) |
            ( static_cast<uint64_t> ( literal[3] ) << 24 ) |
            ( static_cast<uint64_t> ( literal[2] ) << 16 ) |
            ( static_cast<uint64_t> ( literal[1] ) << 8 )  |
            static_cast<uint64_t> ( literal[0] );
    }

    template<class T, uint64_t Magick>
    class Resource
    {
    public:
        virtual ~Resource() {}
        void Load ( uint32_t aId )
        {
            std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
            LoadResource ( aId, buffer.data(), buffer.size() );
            try
            {
                Load ( buffer.data(), buffer.size() );
            }
            catch ( ... )
            {
                Unload();
                throw;
            }
        }

        void Load ( const std::string& aFilename )
        {
            Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
        }

        void Load ( const void* aBuffer, size_t aBufferSize )
        {
            static std::mutex m{};
            static T buffer{};
            std::lock_guard<std::mutex> hold ( m );
            LoadProtoBufObject ( buffer, aBuffer, aBufferSize, Magick );
            Load ( buffer );
            buffer.Clear();
        }
        virtual void Unload () = 0;
    private:
        virtual void Load ( const T& aAnimationMsg ) = 0;
    };
}

#endif

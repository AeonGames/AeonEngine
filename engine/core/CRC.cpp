/*
Copyright (C) 2016,2018,2025 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include "aeongames/CRC.hpp"

namespace AeonGames
{
    /*! \brief Compute the CRC32 of a given message, continuing from a previous CRC value.
        \param message      The message to compute the CRC for.
        \param size         The size of the message in bytes.
        \param previous_crc The previous CRC32 value to continue from.
        \return             The CRC32 of the message combined with the previous CRC.
     */
    uint32_t crc32i ( const char* message, size_t size, uint32_t previous_crc )
    {
        assert ( message != nullptr );
        // Un-finalize the previous CRC to continue computing
        uint32_t       remainder = reflect32<16> ( previous_crc ) ^ 0xFFFFFFFF;
        uint32_t       data;
        /*
         * Divide the message by the polynomial, a byte at a time.
         */
        for ( size_t byte = 0; byte < size; ++byte )
        {
            data = reflect32<4> ( static_cast<uint8_t> ( message[byte] ) ) ^ ( remainder >> ( 32 - 8 ) );
            remainder = crc_table32[data] ^ ( remainder << 8 );
        }
        /*
         * The final remainder is the CRC.
         */
        return reflect32<16> ( remainder ) ^ 0xFFFFFFFF;
    }

    /*! \brief Compute the CRC64 of a given message, continuing from a previous CRC value.
        \param message      The message to compute the CRC for.
        \param size         The size of the message in bytes.
        \param previous_crc The previous CRC64 value to continue from.
        \return             The CRC64 of the message combined with the previous CRC.
     */
    uint64_t crc64i ( const char* message, size_t size, uint64_t previous_crc )
    {
        assert ( message != nullptr );
        // Un-finalize the previous CRC to continue computing
        uint64_t       remainder = reflect64<32> ( previous_crc ) ^ 0xFFFFFFFFFFFFFFFF;
        uint64_t       data;
        /*
         * Divide the message by the polynomial, a byte at a time.
         */
        for ( size_t byte = 0; byte < size; ++byte )
        {
            data = reflect64<4> ( static_cast<uint8_t> ( message[byte] ) ) ^ ( remainder >> ( 64 - 8 ) );
            remainder = crc_table64[data] ^ ( remainder << 8 );
        }
        /*
         * The final remainder is the CRC.
         */
        return reflect64<32> ( remainder ) ^ 0xFFFFFFFFFFFFFFFF;
    }
}

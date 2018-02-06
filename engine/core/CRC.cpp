/*
Copyright (C) 2016,2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/CRC.h"
#include <cassert>

/*! \brief Compute the CRC32 of a given message.
    \return     The CRC32 of the message.
 */
uint32_t crc32i ( const char* message, size_t size )
{
    assert ( message != nullptr );
    uint32_t       remainder = 0xFFFFFFFF;
    uint32_t       data;
    uint8_t        byte;
    /*
     * Divide the message by the polynomial, a byte at a time.
     */
    for ( byte = 0; byte < size; ++byte )
    {
        data = reflect32<4> ( message[byte] ) ^ ( remainder >> ( 32 - 8 ) );
        remainder = crc_table32[data] ^ ( remainder << 8 );
    }
    /*
     * The final remainder is the CRC.
     */
    return reflect32<16> ( remainder ) ^ 0xFFFFFFFF;
}

/*! \brief Compute the CRC64 of a given message.
    \return     The CRC64 of the message.
 */
uint64_t crc64i ( const char* message, size_t size )
{
    assert ( message != nullptr );
    uint64_t       remainder = 0xFFFFFFFFFFFFFFFF;
    uint64_t       data;
    uint8_t        byte;
    /*
     * Divide the message by the polynomial, a byte at a time.
     */
    for ( byte = 0; byte < size; ++byte )
    {
        data = reflect64<4> ( message[byte] ) ^ ( remainder >> ( 64 - 8 ) );
        remainder = crc_table64[data] ^ ( remainder << 8 );
    }
    /*
     * The final remainder is the CRC.
     */
    return reflect64<32> ( remainder ) ^ 0xFFFFFFFFFFFFFFFF;
}

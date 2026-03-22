/*
Copyright (C) 2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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

/** @file Base64.hpp
 *  @brief Base64 encoding and decoding utilities.
 */

#ifndef AEONGAMES_BASE64_H
#define AEONGAMES_BASE64_H

#include "aeongames/Platform.hpp"
#include <string>
#include <string_view>
#include <cstdint>

namespace AeonGames
{
    /**
     * @brief Encode a binary string.
     * @param aData The string to be encoded.
     * @param aDataSize The size of the string.
     * @param aSplit Whether the resulting string should be split each 76 characters.
     * @return The encoded string.
     * @note This implementation skips any invalid characters, not just whitespace.
     */
    DLL std::string Base64Encode ( const uint8_t* aData, size_t aDataSize, bool aSplit = true );
    /**
     * @brief Decode a base64 string.
     * @param aData The base64 string.
     * @param aDataSize The size of the base64 string.
     * @return The decoded string.
     * @note This implementation skips any invalid characters, not just whitespace.
     */
    DLL std::string Base64Decode ( const uint8_t* aData, size_t aDataSize );
    /** @brief Encode a string_view as base64.
     *  @param aData The data to encode.
     *  @return The base64-encoded string.
     */
    DLL std::string Base64Encode ( std::string_view aData );
    /** @brief Decode a base64 string_view.
     *  @param aData The base64 string to decode.
     *  @return The decoded string.
     */
    DLL std::string Base64Decode ( std::string_view aData );
}

#endif
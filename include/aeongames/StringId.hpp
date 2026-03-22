/*
Copyright (C) 2018,2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_STRINGID_H
#define AEONGAMES_STRINGID_H
#include <cstdint>
#include <string>
#include <functional>
#include "aeongames/CRC.hpp"

namespace AeonGames
{
    /** @brief CRC-based compile-time string identifier.
     *
     * Wraps a string literal and its CRC32 hash, enabling efficient
     * compile-time string comparisons and hashing.
     */
    class StringId
    {
    public:
        /** @brief Construct a StringId from a string literal.
         *  @tparam aStringSize Size of the string literal including null terminator.
         *  @param aString The string literal to wrap.
         */
        template<uint32_t aStringSize>
        constexpr StringId ( const char ( &aString ) [aStringSize] ) :
            mString{aString}, mStringSize{aStringSize}, mId{crc32r ( aString ) } {}

        constexpr StringId() noexcept = default;
        /// @brief Copy constructor.
        constexpr StringId ( const StringId& ) noexcept = default;
        /// @brief Move constructor.
        constexpr StringId ( StringId&& aString ) noexcept = default;
        /// @brief Copy assignment operator.
        constexpr StringId& operator= ( const StringId& ) noexcept = default;
        /// @brief Move assignment operator.
        constexpr StringId& operator= ( StringId&& ) noexcept = default;

        /** @brief Get the CRC32 identifier. @return The CRC32 hash of the string. */
        constexpr uint32_t GetId() const
        {
            return mId;
        };
        /** @brief Get the underlying string pointer. @return Pointer to the string literal. */
        constexpr const char* GetString() const
        {
            return mString;
        };
        /** @brief Get the size of the string including null terminator. @return String size. */
        constexpr uint32_t GetStringSize() const
        {
            return mStringSize;
        };
        /** @brief Compare two StringId objects for equality. @param b The other StringId. @return True if IDs match. */
        constexpr bool operator == ( const StringId &b ) const
        {
            return mId == b.mId;
        }
        /** @brief Compare StringId with a raw uint32_t hash. @param b The hash value to compare. @return True if IDs match. */
        constexpr bool operator == ( uint32_t b ) const
        {
            return mId == b;
        }
        /** @brief Convert to uint32_t. @return The CRC32 hash value. */
        constexpr operator uint32_t() const
        {
            return mId;
        }
        /** @brief Convert to std::string. @return A string copy of the wrapped literal. */
        operator std::string() const
        {
            return std::string{mString};
        }
    private:
        const char* mString{};
        uint32_t mStringSize{};
        uint32_t mId{};
    };
}

namespace std
{
    /** @brief std::hash specialization for AeonGames::StringId. */
    template <> struct hash<AeonGames::StringId>
    {
        /** @brief Hash a StringId. @param aStringId The StringId to hash. @return The hash value. */
        constexpr size_t operator() ( const AeonGames::StringId& aStringId ) const
        {
            return aStringId;
        }
    };
}
#endif

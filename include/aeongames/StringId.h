/*
Copyright (C) 2018,2019 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/CRC.h"

namespace AeonGames
{
    class StringId
    {
    public:
        template<uint32_t aStringSize>
        constexpr StringId ( const char ( &aString ) [aStringSize] ) :
            mString{aString}, mStringSize{aStringSize}, mId{crc32r ( aString ) } {}

        constexpr StringId() noexcept = default;
        constexpr StringId ( const StringId& ) noexcept = default;
        constexpr StringId ( StringId&& aString ) noexcept = default;
        constexpr StringId& operator= ( const StringId& ) noexcept = default;
        constexpr StringId& operator= ( StringId&& ) noexcept = default;

        constexpr uint32_t GetId() const
        {
            return mId;
        };
        constexpr const char* GetString() const
        {
            return mString;
        };
        constexpr uint32_t GetStringSize() const
        {
            return mStringSize;
        };
        constexpr bool operator == ( const StringId &b ) const
        {
            return mId == b.mId;
        }
        constexpr bool operator == ( uint32_t b ) const
        {
            return mId == b;
        }
        constexpr operator uint32_t() const
        {
            return mId;
        }
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
    template <> struct hash<AeonGames::StringId>
    {
        constexpr size_t operator() ( const AeonGames::StringId& aStringId ) const
        {
            return aStringId;
        }
    };
}
#endif

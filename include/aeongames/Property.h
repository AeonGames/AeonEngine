/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PROPERTY_H
#define AEONGAMES_PROPERTY_H
#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <sstream>
#include <algorithm>
#include "aeongames/CRC.h"

namespace AeonGames
{
    template<class T>
    class Property
    {
    public:
        Property (
            const std::string& aName,
            const std::string& aDisplayName,
            const std::string& aFormat,
            const std::function<void ( T*, const void* ) >& aSetter,
            const std::function<void ( const T*, void* ) >& aGetter,
            std::initializer_list<Property> aSubProperties = {}
        ) :
            mId{crc32i ( aName.c_str(), aName.size() ) },
            mBufferSize{},
            mName{aName},
            mDisplayName{aDisplayName},
            mFormat{aFormat},
            mSetter{aSetter},
            mGetter{aGetter},
            mParent{},
            mSubProperties{std::move ( aSubProperties ) }
        {
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
            size_t cursor = 0;
            size_t advance = 0;
            size_t size_multiplier = 1;
            while ( cursor < mFormat.size() )
            {
                switch ( mFormat[cursor] )
                {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    size_multiplier = std::stoi ( mFormat.substr ( cursor ), &advance, 10 );
                    cursor += advance;
                    break;
                case 'c':  //char string of length 1
                case 'b':  //signed char    integer
                case 'B':  //unsigned char  integer
                case '?':  //Bool  bool
                    mBufferSize += sizeof ( uint8_t ) * size_multiplier;
                    size_multiplier = 1;
                    ++cursor;
                    break;

                case 'h': //short   integer
                case 'H': //unsigned short  integer
                    mBufferSize += sizeof ( uint16_t ) * size_multiplier;
                    size_multiplier = 1;
                    ++cursor;
                    break;

                case 'i': //int integer
                case 'I': //unsigned int
                case 'l': //long integer
                case 'L': //unsigned long integer
                case 'f': //float float
                    mBufferSize += sizeof ( uint32_t ) * size_multiplier;
                    size_multiplier = 1;
                    ++cursor;
                    break;

                case 'q': //long long integer
                case 'Q': //unsigned long long integer
                case 'd': //double float
                    mBufferSize += sizeof ( uint64_t ) * size_multiplier;
                    size_multiplier = 1;
                    ++cursor;
                    break;
                default:
                {
                    // Lets be pedantic
                    std::ostringstream stream;
                    stream << "Invalid '" << mFormat[cursor] << "' character in format.";
                    throw std::runtime_error ( stream.str().c_str() );
                }
                }
            }
        }
        Property ( const Property& aProperty ) :
            mId{aProperty.mId},
            mBufferSize{aProperty.mBufferSize},
            mName{aProperty.mName},
            mDisplayName{aProperty.mDisplayName},
            mFormat{aProperty.mFormat},
            mSetter{aProperty.mSetter},
            mGetter{aProperty.mGetter},
            mParent{aProperty.mParent},
            mSubProperties{ aProperty.mSubProperties }
        {
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
        }
        Property& operator= ( const Property& aProperty )
        {
            mId = aProperty.mId;
            mBufferSize = aProperty.mBufferSize;
            mName = aProperty.mName;
            mDisplayName = aProperty.mDisplayName;
            mFormat = aProperty.mFormat;
            mSetter = aProperty.mSetter;
            mGetter = aProperty.mGetter;
            mParent = aProperty.mParent;
            mSubProperties = aProperty.mSubProperties;
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
            return *this;
        }
        Property ( const Property&& aProperty ) :
            mId{aProperty.mId},
            mBufferSize{aProperty.mBufferSize},
            mName{std::move ( aProperty.mName ) },
            mDisplayName{std::move ( aProperty.mDisplayName ) },
            mFormat{std::move ( aProperty.mFormat ) },
            mSetter{std::move ( aProperty.mSetter ) },
            mGetter{std::move ( aProperty.mGetter ) },
            mParent{aProperty.mParent},
            mSubProperties{ std::move ( aProperty.mSubProperties ) }
        {
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
        }
        Property& operator= ( const Property&& aProperty )
        {
            mId = aProperty.mId;
            mBufferSize = aProperty.mBufferSize;
            mName = std::move ( aProperty.mName );
            mDisplayName = std::move ( aProperty.mDisplayName );
            mFormat = std::move ( aProperty.mFormat );
            mParent = aProperty.mParent;
            mSubProperties = std::move ( aProperty.mSubProperties );
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
            return *this;
        }
        const size_t GetId() const
        {
            return mId;
        }
        size_t GetBufferSize() const
        {
            return mBufferSize;
        }
        const std::string& GetName() const
        {
            return mName;
        }
        const std::string& GetDisplayName() const
        {
            return mDisplayName;
        }
        const std::string& GetFormat() const
        {
            return mFormat;
        }
        void Set ( T* aInstance, const void* aTuple ) const
        {
            mSetter ( aInstance, aTuple );
        }
        void Get ( const T* aInstance, void* aTuple ) const
        {
            mGetter ( aInstance, aTuple );
        }
        const Property* GetParent() const
        {
            return mParent;
        }
        const std::vector<Property>& GetSubProperties() const
        {
            return mSubProperties;
        }
        size_t GetIndex() const
        {
            auto parent = mParent;
            if ( parent )
            {
                auto index = std::find_if ( parent->GetSubProperties().begin(), parent->GetSubProperties().end(),
                                            [this] ( const Property & property )
                {
                    return &property == this;
                } );
                return index - parent->GetSubProperties().begin();
            }
            throw std::runtime_error ( "Property has no parent and thus no assigned index." );
        }
    private:
        size_t mId {};
        size_t mBufferSize{};
        std::string  mName{};
        std::string  mDisplayName{};
        std::string  mFormat{};
        std::function<void ( T*, const void* ) > mSetter;
        std::function<void ( const T*, void* ) > mGetter;
        Property* mParent{};
        std::vector<Property> mSubProperties{};
    };
}
#endif

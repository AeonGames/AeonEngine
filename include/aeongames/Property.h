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
#include <iostream>
#include <algorithm>
#include <regex>
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
            size_t arg_count = 0;
            std::string regex_match{};
            IterateFormat ( [this, &arg_count, &regex_match] ( size_t aMultiplier, uint8_t aType )
            {
                static const std::string float_pattern{"([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"};
                static const std::string integer_pattern{"([-+]?[0-9]+)"};
                static const std::string whitespace_pattern{"\\s*"};
                static const std::string separator_pattern{"\\s+"};
                switch ( aType )
                {
                case 'c':  //char string of length 1
                case 'b':  //signed char integer
                case 'B':  //unsigned char integer
                case '?':  //Bool bool
                    for ( size_t i = 0; i < aMultiplier; ++i )
                    {
                        if ( arg_count++ )
                        {
                            regex_match += separator_pattern;
                        }
                        regex_match += integer_pattern;
                    }
                    mBufferSize += sizeof ( uint8_t ) * aMultiplier;
                    break;

                case 'h': //short   integer
                case 'H': //unsigned short  integer
                    for ( size_t i = 0; i < aMultiplier; ++i )
                    {
                        if ( arg_count++ )
                        {
                            regex_match += separator_pattern;
                        }
                        regex_match += integer_pattern;
                    }
                    mBufferSize += sizeof ( uint16_t ) * aMultiplier;
                    break;

                case 'i': //int integer
                case 'I': //unsigned int
                case 'l': //long integer
                case 'L': //unsigned long integer
                case 'f': //float float
                    for ( size_t i = 0; i < aMultiplier; ++i )
                    {
                        if ( arg_count++ )
                        {
                            regex_match += separator_pattern;
                        }
                        regex_match += ( ( aType != 'f' ) ? integer_pattern : float_pattern );
                    }
                    mBufferSize += sizeof ( uint32_t ) * aMultiplier;
                    break;

                case 'q': //long long integer
                case 'Q': //unsigned long long integer
                case 'd': //double float
                    for ( size_t i = 0; i < aMultiplier; ++i )
                    {
                        if ( arg_count++ )
                        {
                            regex_match += separator_pattern;
                        }
                        regex_match += ( ( aType != 'd' ) ? integer_pattern : float_pattern );
                    }
                    mBufferSize += sizeof ( uint64_t ) * aMultiplier;
                    break;
                }
                return true;
            } );
            try
            {
                mRegex = std::regex ( regex_match );
            }
            catch ( std::regex_error& e )
            {
                std::cout << "Error: " << e.what() << " at " << __func__ << " line " << __LINE__ << std::endl;
                std::cout << "Regex: " << regex_match << std::endl;
                throw;
            }
        }
        Property ( const Property& aProperty ) :
            mId{aProperty.mId},
            mBufferSize{aProperty.mBufferSize},
            mName{aProperty.mName},
            mDisplayName{aProperty.mDisplayName},
            mFormat{aProperty.mFormat},
            mRegex{aProperty.mRegex},
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
            mRegex = aProperty.mRegex;
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
            mRegex{std::move ( aProperty.mRegex ) },
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
            mRegex = std::move ( aProperty.mRegex );
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
        void SetByString ( T* aInstance, const std::string& aText ) const
        {
            std::vector<uint8_t> tuple ( mBufferSize, 0 );
            uint8_t* write_cursor = tuple.data();
            std::smatch sm;
            if ( std::regex_match ( aText, sm, mRegex ) )
            {
                size_t sm_index = 0;
                IterateFormat ( [&sm, &sm_index, &write_cursor] ( size_t aMultiplier, uint8_t aType )
                {
                    for ( size_t i = 0; i < aMultiplier; ++i )
                    {
                        ++sm_index;
                        switch ( aType )
                        {
                        case 'c':
                            *reinterpret_cast<char*> ( write_cursor ) = std::stoi ( sm[sm_index] );
                            write_cursor += sizeof ( char );
                            break;
                        case 'b':
                            *reinterpret_cast<int8_t*> ( write_cursor ) = std::stoi ( sm[sm_index] );
                            write_cursor += sizeof ( int8_t );
                            break;
                        case 'B':
                        case '?':
                            *reinterpret_cast<uint8_t*> ( write_cursor ) = std::stoi ( sm[sm_index] );
                            write_cursor += sizeof ( uint8_t );
                            break;
                        case 'h':
                            *reinterpret_cast<int16_t*> ( write_cursor ) = std::stoi ( sm[sm_index] );
                            write_cursor += sizeof ( int16_t );
                            break;
                        case 'H':
                            *reinterpret_cast<uint16_t*> ( write_cursor ) = std::stoi ( sm[sm_index] );
                            write_cursor += sizeof ( uint16_t );
                            break;
                        case 'i':
                        case 'l':
                            *reinterpret_cast<int32_t*> ( write_cursor ) = std::stoi ( sm[sm_index] );
                            write_cursor += sizeof ( int32_t );
                            break;
                        case 'I':
                        case 'L':
                            *reinterpret_cast<uint32_t*> ( write_cursor ) = std::stoi ( sm[sm_index] );
                            write_cursor += sizeof ( uint32_t );
                            break;
                        case 'f':
                            *reinterpret_cast<float*> ( write_cursor ) = std::stof ( sm[sm_index] );
                            write_cursor += sizeof ( float );
                            break;
                        case 'q':
                            *reinterpret_cast<int64_t*> ( write_cursor ) = std::stoi ( sm[sm_index] );
                            write_cursor += sizeof ( int64_t );
                            break;
                        case 'Q':
                            *reinterpret_cast<uint64_t*> ( write_cursor ) = std::stoi ( sm[sm_index] );
                            write_cursor += sizeof ( uint64_t );
                            break;
                        case 'd':
                            *reinterpret_cast<double*> ( write_cursor ) = std::stod ( sm[sm_index] );
                            write_cursor += sizeof ( double );
                            break;
                        }
                    }
                    return true;
                } );
                mSetter ( aInstance, tuple.data() );
            }
        }
        void Get ( const T* aInstance, void* aTuple ) const
        {
            mGetter ( aInstance, aTuple );
        }
        std::string GetAsString ( const T* aInstance ) const
        {
            std::vector<uint8_t> tuple ( mBufferSize, 0 );
            mGetter ( aInstance, tuple.data() );
            uint8_t* read_cursor = tuple.data();
            std::string result{};
            size_t needs_separator = 0;
            IterateFormat ( [&read_cursor, &result, &needs_separator] ( size_t aMultiplier, uint8_t aType )
            {
                for ( size_t i = 0; i < aMultiplier; ++i )
                {
                    switch ( aType )
                    {
                    case 'c':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<char*> ( read_cursor ) );
                        read_cursor += sizeof ( char );
                        break;
                    case 'b':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<int8_t*> ( read_cursor ) );
                        read_cursor += sizeof ( int8_t );
                        break;
                    case 'B':
                    case '?':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<uint8_t*> ( read_cursor ) );
                        read_cursor += sizeof ( uint8_t );
                        break;
                    case 'h':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<int16_t*> ( read_cursor ) );
                        read_cursor += sizeof ( int16_t );
                        break;
                    case 'H':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<uint16_t*> ( read_cursor ) );
                        read_cursor += sizeof ( uint16_t );
                        break;
                    case 'i':
                    case 'l':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<int32_t*> ( read_cursor ) );
                        read_cursor += sizeof ( int32_t );
                        break;
                    case 'I':
                    case 'L':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<uint32_t*> ( read_cursor ) );
                        read_cursor += sizeof ( uint32_t );
                        break;
                    case 'f':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<float*> ( read_cursor ) );
                        read_cursor += sizeof ( float );
                        break;
                    case 'q':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<int64_t*> ( read_cursor ) );
                        read_cursor += sizeof ( int64_t );
                        break;
                    case 'Q':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<uint64_t*> ( read_cursor ) );
                        read_cursor += sizeof ( uint64_t );
                        break;
                    case 'd':
                        result += ( needs_separator++ ? " " : "" ) + std::to_string ( *reinterpret_cast<double*> ( read_cursor ) );
                        read_cursor += sizeof ( double );
                        break;
                    }
                }
                return true;
            } );
            return result;
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
        void IterateFormat ( std::function<bool ( size_t, uint8_t ) > aFunction ) const
        {
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
                case 'c':
                case 'b':
                case 'B':
                case '?':
                case 'h':
                case 'H':
                case 'i':
                case 'I':
                case 'l':
                case 'L':
                case 'f':
                case 'q':
                case 'Q':
                case 'd':
                    if ( !aFunction ( size_multiplier, mFormat[cursor] ) )
                    {
                        return;
                    }
                    ++cursor;
                    size_multiplier = 1;
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
        size_t mId {};
        size_t mBufferSize{};
        std::string  mName{};
        std::string  mDisplayName{};
        std::string  mFormat{};
        std::regex   mRegex{};
        std::function<void ( T*, const void* ) > mSetter;
        std::function<void ( const T*, void* ) > mGetter;
        Property* mParent{};
        std::vector<Property> mSubProperties{};
    };
}
#endif

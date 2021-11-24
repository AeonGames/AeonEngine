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
#include "aeongames/Base64.h"
#include <cassert>
#include <cassert>
#include <algorithm>

namespace AeonGames
{
    const static std::string_view Alphabeth{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
    const static char Padding{'='};

    std::string Base64Encode ( const uint8_t* aData, size_t aDataSize )
    {
        assert ( aData );
        assert ( aDataSize );
        std::string result{};
        result.reserve ( ( ( aDataSize + 2 ) / 3 ) * 4 );
        for ( size_t i = 0; i < aDataSize / 3; ++i )
        {
            uint32_t n = ( aData[i * 3] << 16 ) + ( aData[ ( i * 3 ) + 1] << 8 ) + aData[ ( i * 3 ) + 2];
            result.push_back ( Alphabeth[ ( n >> 18 ) & 63] );
            result.push_back ( Alphabeth[ ( n >> 12 ) & 63] );
            result.push_back ( Alphabeth[ ( n >> 6 ) & 63] );
            result.push_back ( Alphabeth[n & 63] );
        }
        switch ( aDataSize % 3 )
        {
        case 1:
        {
            uint32_t n = ( aData[aDataSize - 1] << 16 );
            result.push_back ( Alphabeth[ ( n >> 18 ) & 63] );
            result.push_back ( Alphabeth[ ( n >> 12 ) & 63] );
            result.push_back ( Padding );
            result.push_back ( Padding );
            break;
        }
        case 2:
        {
            uint32_t n = ( aData[aDataSize - 2] << 16 ) + ( aData[aDataSize - 1] << 8 );
            result.push_back ( Alphabeth[ ( n >> 18 ) & 63] );
            result.push_back ( Alphabeth[ ( n >> 12 ) & 63] );
            result.push_back ( Alphabeth[ ( n >> 6 ) & 63] );
            result.push_back ( Padding );
            break;
        }
        }
        return result;
    }

    std::string Base64Decode ( const uint8_t* aData, size_t aDataSize )
    {
        assert ( aData );
        assert ( aDataSize );
        std::string result;
        result.reserve ( ( aDataSize / 4 ) * 3 );
        for ( size_t i = 0; i < aDataSize; i += 4 )
        {
            uint32_t n =
                ( Alphabeth.find ( aData[i + 0] ) << 18 ) +
                ( Alphabeth.find ( aData[i + 1] ) << 12 ) +
                ( Alphabeth.find ( ( aData[i + 2] == Padding ) ? 'A' : aData[i + 2] ) << 6 ) +
                ( Alphabeth.find ( ( aData[i + 3] == Padding ) ? 'A' : aData[i + 3] ) );

            result.push_back ( ( n >> 16 ) & 0xFF );
            result.push_back ( ( n >> 8 ) & 0xFF );
            result.push_back ( n & 0xFF );
        }
        return result;
    }

    std::string Base64Encode ( std::string_view aData )
    {
        return Base64Encode ( reinterpret_cast<const uint8_t*> ( aData.data() ), aData.size() );
    }

    std::string Base64Decode ( std::string_view aData )
    {
        return Base64Decode ( reinterpret_cast<const uint8_t*> ( aData.data() ), aData.size() );
    }
}

/*
Copyright (C) 2018,2025 Rodrigo Jose Hernandez Cordoba

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
#include <vector>
#include <tuple>
#include <string>
#include <functional>
#include <algorithm>
#include <fstream>
#include "aeongames/AeonEngine.hpp"
#include "aeongames/CRC.hpp"
#include <utility>
#ifndef AEONGAMES_DECODER_H
#define AEONGAMES_DECODER_H
namespace AeonGames
{
    template<class T>
    class Decoder
    {
    public:
        static bool Decode ( T& aOutput, uint32_t aId )
        {
            //--------------------------------------------------------
            // File loading code
            //std::ifstream file;
            //file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
            //file.open ( aFileName, std::ios::binary );
            //std::vector<uint8_t> buffer ( (
            //                                  std::istreambuf_iterator<char> ( file ) ),
            //                              ( std::istreambuf_iterator<char>() ) );
            //file.close();
            //--------------------------------------------------------
            size_t buffer_size = GetResourceSize ( aId );
            std::vector<uint8_t> buffer ( buffer_size );
            LoadResource ( aId, buffer.data(), buffer.size() );
            return Decode ( aOutput, buffer.data(), buffer.size() );
        }
        static bool Decode ( T& aOutput, const std::string& aPath )
        {
            return Decode ( aOutput, crc32i ( aPath.data(), aPath.size() ) );
        }

        static bool Decode ( T& aOutput, const void* aBuffer, size_t aBufferSize )
        {
            auto iterator = std::lower_bound ( Decoders.begin(), Decoders.end(), aBuffer,
                                               [] ( const std::tuple<std::string, std::function < bool ( T&, size_t, const void* ) >> & aTuple, const void* aBuffer )
            {
                int difference = std::get<0> ( aTuple ).compare ( 0, std::get<0> ( aTuple ).size(), static_cast<const char*> ( aBuffer ), std::get<0> ( aTuple ).size() );
                return difference < 0;
            } );
            if ( iterator == Decoders.end() || std::get<0> ( *iterator ).compare ( 0, std::get<0> ( *iterator ).size(), static_cast<const char * > ( aBuffer ), std::get<0> ( *iterator ).size() ) != 0 )
            {
                return false;
            }
            return std::get<1> ( *iterator ) ( aOutput, aBufferSize, aBuffer );
        }
        static bool RegisterDecoder ( const std::string& aMagick, const std::function < bool ( T&, size_t, const void* ) > & aDecoder )
        {
            auto iterator = std::lower_bound ( Decoders.begin(), Decoders.end(), aMagick,
                                               [] ( const std::tuple<std::string, std::function < bool ( T&, size_t, const void* ) >> & aTuple, const std::string & aMagick )
            {
                return std::get<0> ( aTuple ) < aMagick;
            } );
            if ( iterator != Decoders.end() && std::get<0> ( *iterator ) == aMagick )
            {
                return false;
            }
            Decoders.insert ( iterator, std::tuple<std::string, std::function < bool ( T&, size_t, const void* ) >> ( aMagick, aDecoder ) );
            return true;
        }
        static bool UnregisterDecoder ( const std::string& aMagick )
        {
            auto iterator = std::lower_bound ( Decoders.begin(), Decoders.end(), aMagick,
                                               [] ( const std::tuple<std::string, std::function < bool ( T&, size_t, const void* ) >>& aTuple, const std::string & aMagick )
            {
                return std::get<0> ( aTuple ) < aMagick;
            } );
            if ( iterator == Decoders.end() )
            {
                return false;
            }
            Decoders.erase ( std::remove_if ( Decoders.begin(), Decoders.end(),
                                              [&aMagick] ( const std::tuple<std::string, std::function < bool ( T&, size_t, const void* ) >> & aTuple )
            {
                return std::get<0> ( aTuple ) == aMagick;
            } ), Decoders.end() );
            return true;
        }
    private:
        static std::vector<std::tuple<std::string, std::function < bool ( T&, size_t, const void* ) >>> Decoders;
    };
    template<class T>
    std::vector < std::tuple<std::string, std::function < bool ( T&, size_t, const void* ) >>> Decoder<T>::Decoders{};
}
#endif

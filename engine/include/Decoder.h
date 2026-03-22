/*
Copyright (C) 2018,2025,2026 Rodrigo Jose Hernandez Cordoba

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
    /** @brief Template class that dispatches decoding of binary data to registered format-specific decoders.
        @tparam T The type of object to decode into. */
    template<class T>
    class Decoder
    {
    public:
        /** @brief Decodes a resource identified by its CRC32 id.
            @param aOutput Object to receive the decoded data.
            @param aId CRC32 identifier of the resource.
            @return true on success, false on failure. */
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
        /** @brief Decodes a resource identified by its path string.
            @param aOutput Object to receive the decoded data.
            @param aPath Path string whose CRC32 is used to locate the resource.
            @return true on success, false on failure. */
        static bool Decode ( T& aOutput, const std::string& aPath )
        {
            return Decode ( aOutput, crc32i ( aPath.data(), aPath.size() ) );
        }

        /** @brief Decodes data from a raw memory buffer by matching its magic bytes to a registered decoder.
            @param aOutput Object to receive the decoded data.
            @param aBuffer Pointer to the data buffer.
            @param aBufferSize Size of the data buffer in bytes.
            @return true on success, false if no matching decoder is found. */
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
        /** @brief Registers a decoder function for the given magic byte sequence.
            @param aMagick Magic byte string identifying the format.
            @param aDecoder Function that decodes data of this format.
            @return true if registration succeeded, false if a decoder for this magic already exists. */
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
        /** @brief Unregisters the decoder for the given magic byte sequence.
            @param aMagick Magic byte string identifying the format to remove.
            @return true if the decoder was found and removed, false otherwise. */
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
        /// @brief Registered decoder functions keyed by magic string.
        static std::vector<std::tuple<std::string, std::function < bool ( T&, size_t, const void* ) >>> Decoders;
    };
    /// @cond INTERNAL
    template<class T>
    std::vector < std::tuple<std::string, std::function < bool ( T&, size_t, const void* ) >>> Decoder<T>::Decoders{};
    /// @endcond
}
#endif

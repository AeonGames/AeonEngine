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

#include <iostream>
#include <cstdint>
#include <functional>
#include <string>
#include "Decoder.h"
#include "gtest/gtest.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( Decoder, HappyPath )
    {
        EXPECT_TRUE ( Decoder<std::string>::RegisterDecoder ( "C)",
                      [] ( std::string & aOutput, size_t aBufferSize, const void* aBuffer )->bool
        {
            bool match = std::string ( "C)" ).compare ( 0, 2, static_cast<const char*> ( aBuffer ), 2 ) == 0;
            EXPECT_TRUE ( match );
            if ( match )
            {
                aOutput = "C Decoder";
            }
            return match;
        } ) );

        EXPECT_TRUE ( Decoder<std::string>::RegisterDecoder ( "A)",
                      [] ( std::string & aOutput, size_t aBufferSize, const void* aBuffer )->bool
        {
            bool match = std::string ( "A)" ).compare ( 0, 2, static_cast<const char*> ( aBuffer ), 2 ) == 0;
            EXPECT_TRUE ( match );
            if ( match )
            {
                aOutput = "A Decoder";
            }
            return match;
        } ) );

        EXPECT_TRUE ( Decoder<std::string>::RegisterDecoder ( "F)",
                      [] ( std::string & aOutput, size_t aBufferSize, const void* aBuffer )->bool
        {
            bool match = std::string ( "F)" ).compare ( 0, 2, static_cast<const char*> ( aBuffer ), 2 ) == 0;
            EXPECT_TRUE ( match );
            if ( match )
            {
                aOutput = "F Decoder";
            }
            return match;
        } ) );

        EXPECT_TRUE ( Decoder<std::string>::RegisterDecoder ( "E)",
                      [] ( std::string & aOutput, size_t aBufferSize, const void* aBuffer )->bool
        {
            bool match = std::string ( "E)" ).compare ( 0, 2, static_cast<const char*> ( aBuffer ), 2 ) == 0;
            EXPECT_TRUE ( match );
            if ( match )
            {
                aOutput = "E Decoder";
            }
            return match;
        } ) );

        std::string output;
        EXPECT_TRUE ( Decoder<std::string>::Decode ( output, strlen ( "C)XXXXXXXXX" ), "C)XXXXXXXXX" ) );
        EXPECT_TRUE ( output == "C Decoder" );
        EXPECT_TRUE ( Decoder<std::string>::Decode ( output, strlen ( "A)XXXXXXXXX" ), "A)XXXXXXXXX" ) );
        EXPECT_TRUE ( output == "A Decoder" );
        EXPECT_TRUE ( Decoder<std::string>::Decode ( output, strlen ( "F)XXXXXXXXX" ), "F)XXXXXXXXX" ) );
        EXPECT_TRUE ( output == "F Decoder" );
        EXPECT_TRUE ( Decoder<std::string>::Decode ( output, strlen ( "E)XXXXXXXXX" ), "E)XXXXXXXXX" ) );
        EXPECT_TRUE ( output == "E Decoder" );

        EXPECT_TRUE ( Decoder<std::string>::UnregisterDecoder ( "F)" ) );
        EXPECT_TRUE ( Decoder<std::string>::UnregisterDecoder ( "E)" ) );
        EXPECT_TRUE ( Decoder<std::string>::UnregisterDecoder ( "C)" ) );
        EXPECT_TRUE ( Decoder<std::string>::UnregisterDecoder ( "A)" ) );
    }
}

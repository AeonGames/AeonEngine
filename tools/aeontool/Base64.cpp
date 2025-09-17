/*
Copyright (C) 2021,2022,2025 Rodrigo Jose Hernandez Cordoba

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


#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <cstring>
#if defined(__unix__) || defined(__MINGW32__)
#include "sys/stat.h"
#endif
#include "Base64.h"
#include "aeongames/Base64.hpp"
#include "aeongames/Pipeline.hpp"
#include <filesystem>

namespace AeonGames
{
    Base64::Base64() = default;
    Base64::~Base64() = default;
    void Base64::ProcessArgs ( int argc, char** argv )
    {
        if ( argc < 3 || ( strcmp ( argv[1], "base64" ) != 0 && ! ( strcmp ( argv[2], "encode" ) == 0 || strcmp ( argv[2], "decode" ) == 0 ) ) )
        {
            std::ostringstream stream;
            stream << "Invalid tool name, expected base64 <encode|decode>, got " << ( ( argc < 2 ) ? "nothing" : argv[1] ) << " " << ( ( argc < 3 ) ? "nothing" : argv[2] ) << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        mDecode = ( strcmp ( argv[2], "decode" ) == 0 );
        for ( int i = 3; i < argc; ++i )
        {
            if ( argv[i][0] == '-' )
            {
                if ( argv[i][1] == '-' )
                {
                    if ( strncmp ( &argv[i][2], "in", sizeof ( "in" ) ) == 0 )
                    {
                        i++;
                        mInputFile = argv[i];
                    }
                    else if ( strncmp ( &argv[i][2], "out", sizeof ( "out" ) ) == 0 )
                    {
                        i++;
                        mOutputFile = argv[i];
                    }
                }
                else
                {
                    switch ( argv[i][1] )
                    {
                    case 'i':
                        i++;
                        mInputFile = argv[i];
                        break;
                    case 'o':
                        i++;
                        mOutputFile = argv[i];
                        break;
                    }
                }
            }
            else
            {
                mInputFile = argv[i];
            }
        }
        if ( mInputFile.empty() )
        {
            throw std::runtime_error ( "No Input file provided." );
        }
        if ( mOutputFile.empty() )
        {
            if ( !mDecode )
            {
                std::filesystem::path input_path ( mInputFile );
                mOutputFile = input_path.replace_extension ( ".b64" ).string();
            }
            else
            {
                throw std::runtime_error ( "No Output file provided. Decoding does not yet infer output extensions." );
            }
        }
    }

    int Base64::operator() ( int argc, char** argv )
    {
        ProcessArgs ( argc, argv );
        std::ifstream in;
        in.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
        in.open ( mInputFile, std::ifstream::in | std::ifstream::binary );
        std::string buffer ( ( std::istreambuf_iterator<char> ( in ) ), ( std::istreambuf_iterator<char>() ) );
        in.close();
        std::string output{};
        if ( mDecode )
        {
            output = Base64Decode ( buffer );
        }
        else
        {
            output = Base64Encode ( buffer );
        }
        std::ofstream out;
        out.exceptions ( std::ofstream::failbit | std::ofstream::badbit );
        out.open ( mOutputFile, std::ofstream::out | std::ofstream::binary );
        out.write ( reinterpret_cast<char*> ( output.data() ), output.size() );
        out.close();
        return 0;
    }
}

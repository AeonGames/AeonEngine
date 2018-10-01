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

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "aeongames/ProtoBufClasses.h"
#include <google/protobuf/text_format.h>
#include "pipeline.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#if defined(__unix__) || defined(__MINGW32__)
#include "sys/stat.h"
#endif
#include "PipelineTool.h"
#include "aeongames/Pipeline.h"

namespace AeonGames
{
    PipelineTool::PipelineTool() = default;
    PipelineTool::~PipelineTool() = default;
    void PipelineTool::ProcessArgs ( int argc, char** argv )
    {
        if ( argc < 2 || ( strcmp ( argv[1], "pipeline" ) != 0 ) )
        {
            std::ostringstream stream;
            stream << "Invalid tool name, expected pipeline, got " << ( ( argc < 2 ) ? "nothing" : argv[1] ) << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        for ( int i = 2; i < argc; ++i )
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
        else if ( mOutputFile.empty() )
        {
            mOutputFile = mInputFile;
        }
    }

    int PipelineTool::operator() ( int argc, char** argv )
    {
        ProcessArgs ( argc, argv );
        std::ifstream file;
        file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
        file.open ( mInputFile, std::ifstream::in | std::ifstream::binary );
        std::vector<uint8_t> buffer ( ( std::istreambuf_iterator<char> ( file ) ), ( std::istreambuf_iterator<char>() ) );
        file.close();
        Pipeline pipeline ( buffer.data(), buffer.size() );
        {
            std::ofstream shader_file ( mOutputFile + ".vert", std::ifstream::out );
            shader_file << pipeline.GetVertexShaderSource() << std::endl;
            shader_file.close();
        }
        {
            std::ofstream shader_file ( mOutputFile + ".frag", std::ifstream::out );
            shader_file << pipeline.GetFragmentShaderSource() << std::endl;
            shader_file.close();
        }
        return 0;
    }
}

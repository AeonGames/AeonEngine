/*
Copyright (C) 2018,2019,2024,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "aeongames/ProtoBufClasses.hpp"
#include <google/protobuf/text_format.h>
#include "pipeline.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <cstring>
#include <filesystem>
#include "PipelineTool.h"
#include "CodeFieldValuePrinter.hpp"
#include "aeongames/Pipeline.hpp"

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

    const std::unordered_map<const char*, std::function<std::string * ( PipelineMsg* ) >> ShaderTypeToExtension
    {
        { ".vert", &PipelineMsg::mutable_vert },
        { ".frag", &PipelineMsg::mutable_frag },
        { ".comp", &PipelineMsg::mutable_comp },
        { ".tesc", &PipelineMsg::mutable_tesc },
        { ".tese", &PipelineMsg::mutable_tese },
        { ".geom", &PipelineMsg::mutable_geom }
    };

    const std::unordered_map<const char*, std::function<const std::string& ( const PipelineMsg* ) >> ShaderTypeToGetter
    {
        { ".vert", &PipelineMsg::vert },
        { ".frag", &PipelineMsg::frag },
        { ".comp", &PipelineMsg::comp },
        { ".tesc", &PipelineMsg::tesc },
        { ".tese", &PipelineMsg::tese },
        { ".geom", &PipelineMsg::geom }
    };

    int PipelineTool::operator() ( int argc, char** argv )
    {
        ProcessArgs ( argc, argv );
        std::filesystem::path input_path{mInputFile};
        std::filesystem::path output_path{mOutputFile};
        char magick_number[8] = "AEONPLN";
        if ( !input_path.has_extension() )
        {
            if ( output_path.extension() != ".pln" && output_path.extension() != ".txt" )
            {
                throw std::runtime_error ( "Unsupported output file extension, must be .pln or .txt" );
            }
            PipelineMsg pipeline_msg;
            bool found_shader{false};
            for ( const auto& [extension, setter] : ShaderTypeToExtension )
            {
                std::filesystem::path shader_path{input_path.replace_extension ( extension ) };
                if ( std::filesystem::exists ( shader_path ) )
                {
                    std::ifstream shader_file ( shader_path );
                    if ( !shader_file )
                    {
                        throw std::runtime_error ( "Failed to open shader file: " + shader_path.string() );
                    }
                    std::string shader_code ( ( std::istreambuf_iterator<char> ( shader_file ) ), std::istreambuf_iterator<char>() );
                    shader_file.close();
                    setter ( &pipeline_msg )->assign ( shader_code );
                    found_shader = true;
                }
            }
            if ( found_shader )
            {
                if ( output_path.extension() == ".pln" )
                {
                    std::ofstream binary_file ( mOutputFile, std::ios::out | std::ios::binary );
                    binary_file << magick_number << '\0';
                    pipeline_msg.SerializeToOstream ( &binary_file );
                    binary_file.close();
                }
                else
                {
                    google::protobuf::TextFormat::Printer printer;
                    for ( int i = 1; i <= pipeline_msg.GetDescriptor()->field_count() ; ++i )
                    {
                        if ( !printer.RegisterFieldValuePrinter (
                                 pipeline_msg.GetDescriptor()->FindFieldByNumber ( i ),
                                 new CodeFieldValuePrinter ) )
                        {
                            std::cout << "Failed to register field value printer." << std::endl;
                        }
                    }
                    std::string text_string;
                    std::ofstream text_file ( mOutputFile, std::ios::out );
                    printer.PrintToString ( pipeline_msg, &text_string );
                    text_file << magick_number << std::endl;
                    text_file.write ( text_string.c_str(), text_string.length() );
                    text_file.close();
                }
            }
            else
            {
                throw std::runtime_error ( "No suitable shader file found." );
            }
        }
        else if ( input_path.extension() == ".pln" || input_path.extension() == ".txt" )
        {
            // Extract shaders from pipeline file
            PipelineMsg pipeline_msg;
            std::ifstream input_file ( mInputFile, input_path.extension() == ".pln" ? ( std::ios::in | std::ios::binary ) : std::ios::in );
            if ( !input_file )
            {
                throw std::runtime_error ( "Failed to open pipeline file: " + mInputFile );
            }

            // Read and verify magic number
            char file_magick[8];
            if ( input_path.extension() == ".pln" )
            {
                input_file.read ( file_magick, 8 );
                if ( strncmp ( file_magick, magick_number, 7 ) != 0 )
                {
                    throw std::runtime_error ( "Invalid pipeline file format (magic number mismatch)" );
                }
                if ( !pipeline_msg.ParseFromIstream ( &input_file ) )
                {
                    throw std::runtime_error ( "Failed to parse binary pipeline file" );
                }
            }
            else // .txt
            {
                input_file.getline ( file_magick, 8 );
                if ( strncmp ( file_magick, magick_number, 7 ) != 0 )
                {
                    throw std::runtime_error ( "Invalid pipeline file format (magic number mismatch)" );
                }
                std::string text_content ( ( std::istreambuf_iterator<char> ( input_file ) ), std::istreambuf_iterator<char>() );
                if ( !google::protobuf::TextFormat::ParseFromString ( text_content, &pipeline_msg ) )
                {
                    throw std::runtime_error ( "Failed to parse text pipeline file" );
                }
            }
            input_file.close();

            // Extract each shader to its own file
            std::filesystem::path base_path = output_path.has_extension() ? output_path.replace_extension() : output_path;
            bool extracted_any = false;

            for ( const auto& [extension, getter] : ShaderTypeToGetter )
            {
                const std::string& shader_code = getter ( &pipeline_msg );
                if ( !shader_code.empty() )
                {
                    std::filesystem::path shader_path = base_path;
                    shader_path.replace_extension ( extension );
                    std::ofstream shader_file ( shader_path, std::ios::out );
                    if ( !shader_file )
                    {
                        throw std::runtime_error ( "Failed to create shader file: " + shader_path.string() );
                    }
                    shader_file << shader_code;
                    shader_file.close();
                    std::cout << "Extracted: " << shader_path.filename().string() << std::endl;
                    extracted_any = true;
                }
            }

            if ( !extracted_any )
            {
                std::cout << "Warning: No shader code found in pipeline file" << std::endl;
            }
        }
        else
        {
            throw std::runtime_error ( "Unsupported file extension" );
        }

        return 0;
    }
}

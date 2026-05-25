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
#include <array>
#include <filesystem>
#include "PipelineTool.h"
#include "CodeFieldValuePrinter.hpp"
#include "aeongames/Pipeline.hpp"

namespace AeonGames
{
    PipelineTool::PipelineTool() = default;
    PipelineTool::~PipelineTool() = default;

    static void PrintPipelineUsage()
    {
        std::cout <<
        "Usage: aeontool pipeline [options]\n"
        "\n"
        "Packs shader source files (.vert, .frag, .comp, .tesc, .tese, .geom)\n"
        "into a pipeline file (.pln binary or .txt text), or extracts the\n"
        "shader sources back from a pipeline file.\n"
        "\n"
        "Options:\n"
        "  -h, --help          Show this help message and exit.\n"
        "  -i, --in  <path>    Input file path.\n"
        "                      If the path has no extension, shader files\n"
        "                      with that base name and the supported shader\n"
        "                      extensions are picked up automatically.\n"
        "                      If the extension is .pln or .txt, the shaders\n"
        "                      are extracted from the pipeline file.\n"
        "  -o, --out <path>    Output file path. Defaults to the input path.\n"
        "                      When packing, must end in .pln or .txt.\n"
        "\n"
        "Per-stage overrides (packing only). These take precedence over\n"
        "base-name auto-discovery and allow several pipelines to share the\n"
        "same source file:\n"
        "      --vert <file>   Vertex shader source.\n"
        "      --frag <file>   Fragment shader source.\n"
        "      --comp <file>   Compute shader source.\n"
        "      --tesc <file>   Tessellation control shader source.\n"
        "      --tese <file>   Tessellation evaluation shader source.\n"
        "      --geom <file>   Geometry shader source.\n"
        "\n"
        "Examples:\n"
        "  aeontool pipeline -i shader -o shader.pln\n"
        "  aeontool pipeline -i shader.pln -o shader\n"
        "  aeontool pipeline --vert no_skeleton.vert --frag shared.frag \\\n"
        "                    -o no_skeleton.txt\n"
                  << std::flush;
    }

    /// Supported shader stage extensions (ordered for deterministic help/iteration).
    static const std::array<const char*, 6> kStageExtensions
    {
        ".vert", ".frag", ".comp", ".tesc", ".tese", ".geom"
    };

    static bool IsStageExtension ( const char* ext )
    {
        for ( const char * known : kStageExtensions )
        {
            if ( strcmp ( ext, known ) == 0 )
            {
                return true;
            }
        }
        return false;
    }

    bool PipelineTool::ProcessArgs ( int argc, char** argv )
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
                    const char* opt = &argv[i][2];
                    if ( strcmp ( opt, "help" ) == 0 )
                    {
                        PrintPipelineUsage();
                        return false;
                    }
                    else if ( strncmp ( opt, "in", sizeof ( "in" ) ) == 0 )
                    {
                        i++;
                        mInputFile = argv[i];
                    }
                    else if ( strncmp ( opt, "out", sizeof ( "out" ) ) == 0 )
                    {
                        i++;
                        mOutputFile = argv[i];
                    }
                    else
                    {
                        // Per-stage overrides: --vert/--frag/--comp/--tesc/--tese/--geom
                        std::string ext = std::string ( "." ) + opt;
                        if ( IsStageExtension ( ext.c_str() ) )
                        {
                            if ( i + 1 >= argc )
                            {
                                throw std::runtime_error ( "Missing value for --" + std::string ( opt ) );
                            }
                            i++;
                            mStageFiles[ext] = argv[i];
                        }
                        else
                        {
                            throw std::runtime_error ( "Unknown option: --" + std::string ( opt ) );
                        }
                    }
                }
                else
                {
                    switch ( argv[i][1] )
                    {
                    case 'h':
                        PrintPipelineUsage();
                        return false;
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
        if ( mInputFile.empty() && mStageFiles.empty() )
        {
            throw std::runtime_error ( "No Input file provided." );
        }
        if ( mOutputFile.empty() )
        {
            if ( mInputFile.empty() )
            {
                throw std::runtime_error ( "No Output file provided. --out is required when only per-stage overrides are used." );
            }
            mOutputFile = mInputFile;
        }
        return true;
    }

    /// @brief Map from shader file extension to its mutable PipelineMsg accessor.
    const std::unordered_map<const char*, std::function<std::string * ( PipelineMsg* ) >> ShaderTypeToExtension
    {
        { ".vert", &PipelineMsg::mutable_vert },
        { ".frag", &PipelineMsg::mutable_frag },
        { ".comp", &PipelineMsg::mutable_comp },
        { ".tesc", &PipelineMsg::mutable_tesc },
        { ".tese", &PipelineMsg::mutable_tese },
        { ".geom", &PipelineMsg::mutable_geom }
    };

    /// @brief Map from shader file extension to its const PipelineMsg getter.
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
        if ( !ProcessArgs ( argc, argv ) )
        {
            return 0;
        }
        std::filesystem::path input_path{mInputFile};
        std::filesystem::path output_path{mOutputFile};
        char magick_number[8] = "AEONPLN";

        const bool packing_mode = !mStageFiles.empty() ||
                                  mInputFile.empty() ||
                                  !input_path.has_extension();

        if ( packing_mode )
        {
            if ( output_path.extension() != ".pln" && output_path.extension() != ".txt" )
            {
                throw std::runtime_error ( "Unsupported output file extension, must be .pln or .txt" );
            }
            PipelineMsg pipeline_msg;
            bool found_shader{false};

            // 1. Base-name auto-discovery (only when input path is an extensionless base).
            if ( !mInputFile.empty() && !input_path.has_extension() )
            {
                for ( const auto& [extension, setter] : ShaderTypeToExtension )
                {
                    if ( mStageFiles.count ( extension ) )
                    {
                        // Will be filled in by per-stage overrides below.
                        continue;
                    }
                    std::filesystem::path shader_path{std::filesystem::path{mInputFile} .replace_extension ( extension ) };
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
            }

            // 2. Per-stage overrides take precedence.
            for ( const auto& [extension, path] : mStageFiles )
            {
                std::function<std::string* ( PipelineMsg* ) > setter;
                for ( const auto& [known_ext, known_setter] : ShaderTypeToExtension )
                {
                    if ( extension == known_ext )
                    {
                        setter = known_setter;
                        break;
                    }
                }
                if ( !setter )
                {
                    throw std::runtime_error ( "Unknown stage extension: " + extension );
                }
                std::ifstream shader_file ( path );
                if ( !shader_file )
                {
                    throw std::runtime_error ( "Failed to open shader file: " + path );
                }
                std::string shader_code ( ( std::istreambuf_iterator<char> ( shader_file ) ), std::istreambuf_iterator<char>() );
                shader_file.close();
                setter ( &pipeline_msg )->assign ( shader_code );
                found_shader = true;
            }

            if ( found_shader )
            {
                if ( output_path.extension() == ".pln" )
                {
                    std::ofstream binary_file ( mOutputFile, std::ios::out | std::ios::binary );
                    binary_file << magick_number << '\0';
                    if ( !pipeline_msg.SerializeToOstream ( &binary_file ) )
                    {
                        std::cerr << "Failed to serialize pipeline message to binary format.";
                        throw std::runtime_error ( "Failed to serialize pipeline message to binary format." );
                    }
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
                    if ( !printer.PrintToString ( pipeline_msg, &text_string ) )
                    {
                        std::cerr << "Failed to serialize pipeline message to text format.";
                        throw std::runtime_error ( "Failed to serialize pipeline message to text format." );
                    }
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

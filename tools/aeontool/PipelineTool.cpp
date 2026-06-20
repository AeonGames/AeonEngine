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
#include <unordered_set>
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
        "Per-renderer variants. A stage may carry renderer-scoped sources keyed\n"
        "by a comma-separated renderer set (the names renderer plugins export,\n"
        "e.g. \"Vulkan\" or \"Vulkan,Metal\"). A renderer not named in any set uses\n"
        "the default (unscoped) source. The selector sets of a stage must be\n"
        "pairwise disjoint.\n"
        "      --variant <stage> <set> <file>  Source for the renderer set.\n"
        "      --disable <stage> <set>         Disable the stage for the set.\n"
        "\n"
        "Examples:\n"
        "  aeontool pipeline -i shader -o shader.pln\n"
        "  aeontool pipeline -i shader.pln -o shader\n"
        "  aeontool pipeline --vert no_skeleton.vert --frag shared.frag \\\n"
        "                    -o no_skeleton.txt\n"
        "  aeontool pipeline --vert ps.vert --geom ps.geom --frag ps.frag \\\n"
        "                    --variant vert Vulkan ps_mv.vert --disable geom Vulkan \\\n"
        "                    -o point_shadow_depth.txt\n"
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
                    else if ( strcmp ( opt, "variant" ) == 0 )
                    {
                        // --variant <stage> <selector> <file>: a renderer-scoped
                        // stage source. <stage> is the bare name (vert, geom, ...),
                        // <selector> a comma-separated renderer set.
                        if ( i + 3 >= argc )
                        {
                            throw std::runtime_error ( "--variant requires <stage> <selector> <file>" );
                        }
                        std::string ext = std::string ( "." ) + argv[i + 1];
                        if ( !IsStageExtension ( ext.c_str() ) )
                        {
                            throw std::runtime_error ( "Unknown stage for --variant: " + std::string ( argv[i + 1] ) );
                        }
                        mStageFiles.push_back ( { ext, argv[i + 2], argv[i + 3], false } );
                        i += 3;
                    }
                    else if ( strcmp ( opt, "disable" ) == 0 )
                    {
                        // --disable <stage> <selector>: turn a stage explicitly off
                        // for a renderer set (overrides the default without inheriting).
                        if ( i + 2 >= argc )
                        {
                            throw std::runtime_error ( "--disable requires <stage> <selector>" );
                        }
                        std::string ext = std::string ( "." ) + argv[i + 1];
                        if ( !IsStageExtension ( ext.c_str() ) )
                        {
                            throw std::runtime_error ( "Unknown stage for --disable: " + std::string ( argv[i + 1] ) );
                        }
                        mStageFiles.push_back ( { ext, argv[i + 2], std::string{}, true } );
                        i += 2;
                    }
                    else
                    {
                        // Per-stage overrides: --vert/--frag/--comp/--tesc/--tese/--geom
                        // (default variant, empty selector).
                        std::string ext = std::string ( "." ) + opt;
                        if ( IsStageExtension ( ext.c_str() ) )
                        {
                            if ( i + 1 >= argc )
                            {
                                throw std::runtime_error ( "Missing value for --" + std::string ( opt ) );
                            }
                            i++;
                            mStageFiles.push_back ( { ext, std::string{}, argv[i], false } );
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

    /// @brief Map from shader file extension to a setter for its default
    /// (renderer-agnostic) variant. Each stage is now a map keyed by a renderer
    /// selector; the empty key "" is the default. Compute shaders are handled
    /// separately because their value is an ordered stage list.
    const std::unordered_map<const char*, std::function<std::string * ( PipelineMsg* ) >> ShaderTypeToExtension
    {
        { ".vert", [] ( PipelineMsg * m ) -> std::string* { return & ( *m->mutable_vert() ) [""]; } },
        { ".frag", [] ( PipelineMsg * m ) -> std::string* { return & ( *m->mutable_frag() ) [""]; } },
        { ".tesc", [] ( PipelineMsg * m ) -> std::string* { return & ( *m->mutable_tesc() ) [""]; } },
        { ".tese", [] ( PipelineMsg * m ) -> std::string* { return & ( *m->mutable_tese() ) [""]; } },
        { ".geom", [] ( PipelineMsg * m ) -> std::string* { return & ( *m->mutable_geom() ) [""]; } }
    };

    /// @brief Map from shader file extension to a getter for its default
    /// (empty-key) variant, or an empty string when absent.
    const std::unordered_map<const char*, std::function<const std::string& ( const PipelineMsg* ) >> ShaderTypeToGetter
    {
        { ".vert", [] ( const PipelineMsg * m ) -> const std::string& { static const std::string e; auto it = m->vert().find ( "" ); return it != m->vert().end() ? it->second : e; } },
        { ".frag", [] ( const PipelineMsg * m ) -> const std::string& { static const std::string e; auto it = m->frag().find ( "" ); return it != m->frag().end() ? it->second : e; } },
        { ".tesc", [] ( const PipelineMsg * m ) -> const std::string& { static const std::string e; auto it = m->tesc().find ( "" ); return it != m->tesc().end() ? it->second : e; } },
        { ".tese", [] ( const PipelineMsg * m ) -> const std::string& { static const std::string e; auto it = m->tese().find ( "" ); return it != m->tese().end() ? it->second : e; } },
        { ".geom", [] ( const PipelineMsg * m ) -> const std::string& { static const std::string e; auto it = m->geom().find ( "" ); return it != m->geom().end() ? it->second : e; } }
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

            // Helper: read a shader source file in full.
            auto read_shader = [] ( const std::filesystem::path & shader_path ) -> std::string
            {
                std::ifstream shader_file ( shader_path );
                if ( !shader_file )
                {
                    throw std::runtime_error ( "Failed to open shader file: " + shader_path.string() );
                }
                return std::string ( ( std::istreambuf_iterator<char> ( shader_file ) ), std::istreambuf_iterator<char>() );
            };

            // Helper: was this stage's DEFAULT variant provided as an override?
            // Only a default (unscoped, non-disabled) override suppresses
            // base-name auto-discovery of that stage; renderer-scoped variants
            // sit alongside the auto-discovered default.
            auto stage_overridden = [this] ( const std::string & ext ) -> bool
            {
                for ( const auto& sf : mStageFiles )
                {
                    if ( sf.extension == ext && sf.selector.empty() && !sf.disabled )
                    {
                        return true;
                    }
                }
                return false;
            };

            // Helper: target string for a graphics stage's variant keyed by the
            // renderer selector ("" = default). Creates the entry if absent.
            auto stage_setter = [&pipeline_msg] ( const std::string & ext, const std::string & selector ) -> std::string *
            {
                if ( ext == ".vert" )
                {
                    return & ( *pipeline_msg.mutable_vert() ) [selector];
                }
                if ( ext == ".frag" )
                {
                    return & ( *pipeline_msg.mutable_frag() ) [selector];
                }
                if ( ext == ".tesc" )
                {
                    return & ( *pipeline_msg.mutable_tesc() ) [selector];
                }
                if ( ext == ".tese" )
                {
                    return & ( *pipeline_msg.mutable_tese() ) [selector];
                }
                if ( ext == ".geom" )
                {
                    return & ( *pipeline_msg.mutable_geom() ) [selector];
                }
                return nullptr;
            };

            // 1. Base-name auto-discovery (only when input path is an extensionless base).
            if ( !mInputFile.empty() && !input_path.has_extension() )
            {
                for ( const auto& [extension, setter] : ShaderTypeToExtension )
                {
                    if ( stage_overridden ( extension ) )
                    {
                        // Will be filled in by per-stage overrides below.
                        continue;
                    }
                    std::filesystem::path shader_path{std::filesystem::path{mInputFile} .replace_extension ( extension ) };
                    if ( std::filesystem::exists ( shader_path ) )
                    {
                        setter ( &pipeline_msg )->assign ( read_shader ( shader_path ) );
                        found_shader = true;
                    }
                }
                // Compute stage auto-discovery (single base.comp); multiple compute
                // stages must be provided explicitly via repeated --comp overrides.
                if ( !stage_overridden ( ".comp" ) )
                {
                    std::filesystem::path shader_path{std::filesystem::path{mInputFile} .replace_extension ( ".comp" ) };
                    if ( std::filesystem::exists ( shader_path ) )
                    {
                        ( *pipeline_msg.mutable_comp() ) [""].add_stage ( read_shader ( shader_path ) );
                        found_shader = true;
                    }
                }
            }

            // 2. Per-stage overrides take precedence and preserve command-line order.
            for ( const auto& sf : mStageFiles )
            {
                if ( sf.extension == ".comp" )
                {
                    // Compute stages are ordered; append in command-line order. A
                    // disabled compute selector leaves an empty stage list.
                    auto& stages = ( *pipeline_msg.mutable_comp() ) [sf.selector];
                    if ( !sf.disabled )
                    {
                        stages.add_stage ( read_shader ( sf.path ) );
                    }
                    found_shader = true;
                    continue;
                }
                std::string* target = stage_setter ( sf.extension, sf.selector );
                if ( !target )
                {
                    throw std::runtime_error ( "Unknown stage extension: " + sf.extension );
                }
                // A disabled stage keeps an empty value: the selector overrides
                // the default without inheriting it.
                if ( sf.disabled )
                {
                    target->clear();
                }
                else
                {
                    target->assign ( read_shader ( sf.path ) );
                }
                found_shader = true;
            }

            // Renderer selectors of a stage must be pairwise disjoint so at most
            // one matches any renderer; reject overlaps so an ambiguous pipeline
            // can never be packed.
            auto validate_disjoint = [] ( const auto & aStageMap, const char* aStageName )
            {
                std::unordered_set<std::string> seen;
                for ( const auto& entry : aStageMap )
                {
                    const std::string& key = entry.first;
                    if ( key.empty() )
                    {
                        continue;
                    }
                    size_t start = 0;
                    while ( true )
                    {
                        const size_t comma = key.find ( ',', start );
                        const size_t end = ( comma == std::string::npos ) ? key.size() : comma;
                        std::string name = key.substr ( start, end - start );
                        if ( !name.empty() && !seen.insert ( name ).second )
                        {
                            throw std::runtime_error ( std::string ( "Renderer '" ) + name +
                                                       "' appears in multiple " + aStageName + " selectors" );
                        }
                        if ( comma == std::string::npos )
                        {
                            break;
                        }
                        start = comma + 1;
                    }
                }
            };
            validate_disjoint ( pipeline_msg.vert(), "vert" );
            validate_disjoint ( pipeline_msg.frag(), "frag" );
            validate_disjoint ( pipeline_msg.comp(), "comp" );
            validate_disjoint ( pipeline_msg.tesc(), "tesc" );
            validate_disjoint ( pipeline_msg.tese(), "tese" );
            validate_disjoint ( pipeline_msg.geom(), "geom" );

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
                    // Open in binary mode so newlines are written as LF rather
                    // than being translated to CRLF by the Windows CRT.
                    std::ofstream text_file ( mOutputFile, std::ios::out | std::ios::binary );
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
                    std::ofstream shader_file ( shader_path, std::ios::out | std::ios::binary );
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

            // Extract the default compute stages (empty-key variant). Multiple
            // stages get an index suffix so they land on distinct files.
            const auto comp_it = pipeline_msg.comp().find ( "" );
            const int comp_count = ( comp_it != pipeline_msg.comp().end() ) ? comp_it->second.stage_size() : 0;
            for ( int i = 0; i < comp_count; ++i )
            {
                const std::string& shader_code = comp_it->second.stage ( i );
                if ( shader_code.empty() )
                {
                    continue;
                }
                std::filesystem::path shader_path = base_path;
                if ( comp_count > 1 )
                {
                    shader_path = base_path.string() + "." + std::to_string ( i ) + ".comp";
                }
                else
                {
                    shader_path.replace_extension ( ".comp" );
                }
                std::ofstream shader_file ( shader_path, std::ios::out | std::ios::binary );
                if ( !shader_file )
                {
                    throw std::runtime_error ( "Failed to create shader file: " + shader_path.string() );
                }
                shader_file << shader_code;
                shader_file.close();
                std::cout << "Extracted: " << shader_path.filename().string() << std::endl;
                extracted_any = true;
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

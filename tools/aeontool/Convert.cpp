/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include "material.pb.h"
#include "mesh.pb.h"
#include "skeleton.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <regex>
#if defined(__unix__) || defined(__MINGW32__)
#include "sys/stat.h"
#endif
#include "Convert.h"

/** @todo This code may benefit from ProtoBufHelpers.h,
    but by doing so, it must become public
    and aeontool would need to link
    against the AeonEngine library just for FileExists.
    Find a workaround?.*/

namespace AeonGames
{
    static const char float_pattern[] = "([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)";
    static const char int_pattern[] = "([-+]?[0-9]*)";
    static const char whitespace_pattern[] = "\\s*";
    static const char separator_pattern[] = "\\s+";
    uint32_t GetStride ( const MeshBuffer& aMeshBuffer )
    {
        uint32_t stride = 0;
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_POSITION_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_NORMAL_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_TANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_BITANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_UV_BIT )
        {
            stride += sizeof ( float ) * 2;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_WEIGHT_BIT )
        {
            stride += sizeof ( uint8_t ) * 8;
        }
        return stride;
    }
    std::string GetVertexBufferRegexPattern ( const MeshBuffer& aMeshBuffer )
    {
        std::string pattern{"\\(\\s*"};
        bool want_initial_separator = false;
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_POSITION_BIT )
        {
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            want_initial_separator = true;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_NORMAL_BIT )
        {
            if ( want_initial_separator )
            {
                pattern += separator_pattern;
            }
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            want_initial_separator = true;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_TANGENT_BIT )
        {
            if ( want_initial_separator )
            {
                pattern += separator_pattern;
            }
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            want_initial_separator = true;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_BITANGENT_BIT )
        {
            if ( want_initial_separator )
            {
                pattern += separator_pattern;
            }
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            want_initial_separator = true;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_UV_BIT )
        {
            if ( want_initial_separator )
            {
                pattern += separator_pattern;
            }
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            want_initial_separator = true;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_FlagMask_WEIGHT_BIT )
        {
            if ( want_initial_separator )
            {
                pattern += separator_pattern;
            }
            pattern += int_pattern;
            pattern += separator_pattern;
            pattern += int_pattern;
            pattern += separator_pattern;
            pattern += int_pattern;
            pattern += separator_pattern;
            pattern += int_pattern;
        }
        pattern += "\\s*\\)";
        return pattern;
    }
    class CodeFieldValuePrinter : public google::protobuf::TextFormat::FieldValuePrinter
    {
    public:
        CodeFieldValuePrinter() : google::protobuf::TextFormat::FieldValuePrinter()
        {
        };
        std::string PrintString ( const std::string & val ) const override
        {
            std::string pattern ( "\\\\n" );
            std::string format ( "$&\"\n\"" );
            std::regex newline ( pattern );
            std::string printed ( google::protobuf::TextFormat::FieldValuePrinter::PrintString ( val ) );
            printed = std::regex_replace ( printed, newline, format );
            return printed;
        }
    };

    class VertexBufferFieldValuePrinter : public google::protobuf::TextFormat::FieldValuePrinter
    {
    public:
        VertexBufferFieldValuePrinter ( const MeshBuffer& aMeshBuffer ) :
            google::protobuf::TextFormat::FieldValuePrinter(),
            mMeshBuffer{aMeshBuffer}
        {
        };
        std::string PrintBytes ( const std::string & val ) const override
        {
            const uint8_t* cursor = reinterpret_cast<const uint8_t*> ( val.data() );
            std::ostringstream stream;
            stream << std::endl;
            for ( std::size_t i = 0; i < mMeshBuffer.vertexcount(); ++i )
            {
                stream << "\"(";
                if ( mMeshBuffer.vertexflags() & MeshBuffer_FlagMask_POSITION_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream  << " " << values[0] << " " << values[1] << " " << values[2];
                    cursor += sizeof ( float ) * 3;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_FlagMask_NORMAL_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream  << " " << values[0] << " " << values[1] << " " << values[2];
                    cursor += sizeof ( float ) * 3;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_FlagMask_TANGENT_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream  << " " << values[0] << " " << values[1] << " " << values[2];
                    cursor += sizeof ( float ) * 3;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_FlagMask_BITANGENT_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream  << " " << values[0] << " " << values[1] << " " << values[2];
                    cursor += sizeof ( float ) * 3;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_FlagMask_UV_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream  << " " << values[0] << " " << values[1];
                    cursor += sizeof ( float ) * 2;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_FlagMask_WEIGHT_BIT )
                {
                    const uint8_t* values = cursor;
                    stream  << " " <<
                            static_cast<uint32_t> ( values[0] ) << " " <<
                            static_cast<uint32_t> ( values[1] ) << " " <<
                            static_cast<uint32_t> ( values[2] ) << " " <<
                            static_cast<uint32_t> ( values[3] ) << " " <<
                            static_cast<uint32_t> ( values[4] ) << " " <<
                            static_cast<uint32_t> ( values[5] ) << " " <<
                            static_cast<uint32_t> ( values[6] ) << " " <<
                            static_cast<uint32_t> ( values[7] );
                    cursor += sizeof ( uint8_t ) * 8;
                }
                stream << " )\"" << std::endl;
            }
            return stream.str();
        }
    private:
        const MeshBuffer& mMeshBuffer;
    };

    Convert::Convert ( int argc, char** argv )
    {
        for ( int i = 1; i < argc; ++i )
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
            ///@todo Try to guess correct output extension based on the input extension.
            mOutputFile = mInputFile + ".out";
        }
    }
    Convert::~Convert()
        = default;
    int Convert::Run()
    {
        PipelineBuffer pipeline_buffer;
        MaterialBuffer material_buffer;
        MeshBuffer mesh_buffer;
        SkeletonBuffer skeleton_buffer;
        ::google::protobuf::Message* message = nullptr;
        char magick_number[8] = { 0 };
        bool binary_input = false;
        {
            // Try to read input file
            struct stat stat_buffer;
            if ( stat ( mInputFile.c_str(), &stat_buffer ) != 0 )
            {
                std::ostringstream stream;
                stream << "File " << mInputFile << " Not Found (error code:" << errno << ")";
                throw std::runtime_error ( stream.str().c_str() );
            }
            std::ifstream file;
            file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
            file.open ( mInputFile, std::ifstream::in | std::ifstream::binary );
            file.read ( magick_number, sizeof ( magick_number ) );
            file.exceptions ( std::ifstream::badbit );

            // Determine file properties
            switch ( GetFileType ( magick_number ) )
            {
            /* coverity[unterminated_case] */
            case FileType::AEONPRGB:
                binary_input = true;
            /* coverity[fallthrough] */
            case FileType::AEONPRGT:
                message = &pipeline_buffer;
                break;
            /* coverity[unterminated_case] */
            case FileType::AEONMTLB:
                binary_input = true;
            /* coverity[fallthrough] */
            case FileType::AEONMTLT:
                message = &material_buffer;
                break;
            /* coverity[unterminated_case] */
            case FileType::AEONMSHB:
                binary_input = true;
            /* coverity[fallthrough] */
            case FileType::AEONMSHT:
                message = &mesh_buffer;
                break;
            /* coverity[unterminated_case] */
            case FileType::AEONSKLB:
                binary_input = true;
            /* coverity[fallthrough] */
            case FileType::AEONSKLT:
                message = &skeleton_buffer;
                break;
            default:
                file.close();
                std::ostringstream stream;
                stream << "File" << mInputFile << " is not in a valid AeonGames format.";
                throw std::runtime_error ( stream.str().c_str() );
            }

            assert ( message && "Message is null." );

            // Read and parse Input
            if ( binary_input )
            {
                if ( !message->ParseFromIstream ( &file ) )
                {
                    throw std::runtime_error ( "Binary file parsing failed." );
                }
            }
            else
            {
                google::protobuf::TextFormat::Parser parser;
                std::string text ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
                if ( !parser.ParseFromString (
                         text,
                         message ) )
                {
                    throw std::runtime_error ( "Text file parsing failed." );
                }
                if ( message == &mesh_buffer )
                {
                    /* Presume raw buffer data if the lenght of the vertex buffer
                     string exactly maches the string length. */
                    if ( GetStride ( mesh_buffer ) *mesh_buffer.vertexcount() != mesh_buffer.vertexbuffer().size() )
                    {
                        std::string vertex_buffer;
                        std::smatch match_results;
                        std::string vertex_string{mesh_buffer.vertexbuffer() };
                        std::regex vertex_regex ( GetVertexBufferRegexPattern ( mesh_buffer ) );
                        while ( std::regex_search ( vertex_string, match_results, vertex_regex ) )
                        {
                            float float_value;
                            uint8_t uint8_t_value;
                            size_t index = 1;
                            if ( mesh_buffer.vertexflags() & MeshBuffer_FlagMask_POSITION_BIT )
                            {
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                            }
                            if ( mesh_buffer.vertexflags() & MeshBuffer_FlagMask_NORMAL_BIT )
                            {
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                            }
                            if ( mesh_buffer.vertexflags() & MeshBuffer_FlagMask_TANGENT_BIT )
                            {
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                            }
                            if ( mesh_buffer.vertexflags() & MeshBuffer_FlagMask_BITANGENT_BIT )
                            {
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                            }
                            if ( mesh_buffer.vertexflags() & MeshBuffer_FlagMask_UV_BIT )
                            {
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                                float_value = std::stof ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                            }
                            if ( mesh_buffer.vertexflags() & MeshBuffer_FlagMask_WEIGHT_BIT )
                            {
                                uint8_t_value = std::stoi ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                                uint8_t_value = std::stoi ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                                uint8_t_value = std::stoi ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                                uint8_t_value = std::stoi ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                                uint8_t_value = std::stoi ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                                uint8_t_value = std::stoi ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                                uint8_t_value = std::stoi ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                                uint8_t_value = std::stoi ( match_results[index++] );
                                vertex_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                            }
                            vertex_string = match_results.suffix();
                        }
                        mesh_buffer.set_vertexbuffer ( std::move ( vertex_buffer ) );
                    }
                }
            }
            file.close();
        }
        // Write Output
        {
            if ( binary_input )
            {
                // Write Text Version
                google::protobuf::TextFormat::Printer printer;

                // Try to print shader code in a more human readable format.
                if ( ( message == &pipeline_buffer ) && ( !printer.RegisterFieldValuePrinter (
                            pipeline_buffer.vertex_shader().GetDescriptor()->FindFieldByName ( "code" ),
                            new CodeFieldValuePrinter ) ) )
                {
                    std::cout << "Failed to register field value printer." << std::endl;
                }

                // Print Vertex buffers in a more human readable format.
                if ( ( message == &mesh_buffer ) && ( !printer.RegisterFieldValuePrinter (
                        mesh_buffer.GetDescriptor()->FindFieldByName ( "VertexBuffer" ),
                        new VertexBufferFieldValuePrinter ( mesh_buffer ) ) ) )
                {
                    std::cout << "Failed to register vertex buffer printer." << std::endl;
                }

                std::string text_string;
                std::ofstream text_file ( mOutputFile, std::ifstream::out );
                printer.PrintToString ( *message, &text_string );
                text_file << magick_number << std::endl;
                text_file.write ( text_string.c_str(), text_string.length() );
                text_file.close();
            }
            else
            {
                std::ofstream binary_file ( mOutputFile, std::ifstream::out | std::ifstream::binary );
                magick_number[7] = '\0';
                binary_file << magick_number << '\0';
                message->SerializeToOstream ( &binary_file );
                binary_file.close();
            }
        }
        return 0;
    }
    Convert::FileType Convert::GetFileType ( const char * aMagic ) const
    {
        Convert::FileType retval = Convert::FileType::UNKNOWN;
        if ( strncmp ( aMagic, "AEON", 4 ) == 0 )
        {
            const char* type = aMagic + 4;
            // This looks like an AeonEngine file, now determine which one.
            if ( strncmp ( type, "PRG", 3 ) == 0 )
            {
                retval = ( type[3] == '\0' ) ? Convert::FileType::AEONPRGB :
                         Convert::FileType::AEONPRGT;
            }
            else if ( strncmp ( type, "MTL", 3 ) == 0 )
            {
                retval = ( type[3] == '\0' ) ? Convert::FileType::AEONMTLB :
                         Convert::FileType::AEONMTLT;
            }
            else if ( strncmp ( type, "MSH", 3 ) == 0 )
            {
                retval = ( type[3] == '\0' ) ? Convert::FileType::AEONMSHB :
                         Convert::FileType::AEONMSHT;
            }
        }
        return retval;
    }
}

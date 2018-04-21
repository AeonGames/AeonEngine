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
    static const char int_pattern[] = "([-+]?[0-9]+)";
    static const char whitespace_pattern[] = "\\s*";
    static const char separator_pattern[] = "\\s+";
    uint32_t GetStride ( const MeshBuffer& aMeshBuffer )
    {
        uint32_t stride = 0;
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_POSITION_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_NORMAL_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_TANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_BITANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_UV_BIT )
        {
            stride += sizeof ( float ) * 2;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_WEIGHT_BIT )
        {
            stride += sizeof ( uint8_t ) * 8;
        }
        return stride;
    }
    std::string GetVertexBufferRegexPattern ( const MeshBuffer& aMeshBuffer )
    {
        std::string pattern{ "\\(\\s*" };
        bool want_initial_separator = false;
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_POSITION_BIT )
        {
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            pattern += separator_pattern;
            pattern += float_pattern;
            want_initial_separator = true;
        }
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_NORMAL_BIT )
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
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_TANGENT_BIT )
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
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_BITANGENT_BIT )
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
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_UV_BIT )
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
        if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_WEIGHT_BIT )
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
            try
            {
                std::regex newline ( pattern );
                std::string printed ( google::protobuf::TextFormat::FieldValuePrinter::PrintString ( val ) );
                printed = std::regex_replace ( printed, newline, format );
                return printed;
            }
            catch ( std::regex_error& e )
            {
                std::cout << "Error: " << e.what() << " at " << __func__ << " line " << __LINE__ << std::endl;
                throw;
            }
        }
    };

    class VertexBufferFieldValuePrinter : public google::protobuf::TextFormat::FieldValuePrinter
    {
    public:
        VertexBufferFieldValuePrinter ( const MeshBuffer& aMeshBuffer ) :
            google::protobuf::TextFormat::FieldValuePrinter(),
            mMeshBuffer{ aMeshBuffer }
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
                if ( mMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_POSITION_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream << " " << values[0] << " " << values[1] << " " << values[2];
                    cursor += sizeof ( float ) * 3;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_NORMAL_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream << " " << values[0] << " " << values[1] << " " << values[2];
                    cursor += sizeof ( float ) * 3;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_TANGENT_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream << " " << values[0] << " " << values[1] << " " << values[2];
                    cursor += sizeof ( float ) * 3;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_BITANGENT_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream << " " << values[0] << " " << values[1] << " " << values[2];
                    cursor += sizeof ( float ) * 3;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_UV_BIT )
                {
                    const float* values = reinterpret_cast<const float*> ( cursor );
                    stream << " " << values[0] << " " << values[1];
                    cursor += sizeof ( float ) * 2;
                }
                if ( mMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_WEIGHT_BIT )
                {
                    const uint8_t* values = cursor;
                    stream << " " <<
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

    class IndexBufferFieldValuePrinter : public google::protobuf::TextFormat::FieldValuePrinter
    {
    public:
        IndexBufferFieldValuePrinter ( const MeshBuffer& aMeshBuffer ) :
            google::protobuf::TextFormat::FieldValuePrinter(),
            mMeshBuffer{ aMeshBuffer }
        {
        };
        std::string PrintBytes ( const std::string & val ) const override
        {
            const uint8_t* cursor8;
            const uint16_t* cursor16;
            const uint32_t* cursor32 =
                reinterpret_cast<const uint32_t*> ( cursor16 = reinterpret_cast<const uint16_t*> ( cursor8 = reinterpret_cast<const uint8_t*> ( val.data() ) ) );
            std::ostringstream stream;
            stream << std::endl;
            for ( size_t i = 0; i < mMeshBuffer.indexcount(); ++i )
            {
                if ( i % 3 == 0 )
                {
                    stream << "\"";
                }
                switch ( mMeshBuffer.indextype() )
                {
                case MeshBuffer_IndexTypeEnum_UNSIGNED_BYTE:
                    stream << static_cast<uint32_t> ( cursor8[i] );
                    break;
                case MeshBuffer_IndexTypeEnum_UNSIGNED_SHORT:
                    stream << static_cast<uint32_t> ( cursor16[i] );
                    break;
                case MeshBuffer_IndexTypeEnum_UNSIGNED_INT:
                    stream << cursor32[i];
                    break;
                default:
                    // I am deprecating any index types not supported by the official exporter.
                    throw std::runtime_error ( "Unknown or deprecated index buffer type." );
                }
                if ( ( i + 1 ) % 3 == 0 )
                {
                    stream << " \"" << std::endl;
                }
                else
                {
                    stream << " ";
                }
            }
            return stream.str();
        }
    private:
        const MeshBuffer& mMeshBuffer;
    };

    std::string ParseVertexBuffer ( const MeshBuffer& aMeshBuffer )
    {
        std::string vertex_buffer;
        std::smatch match_results;
        std::string vertex_string{ aMeshBuffer.vertexbuffer() };
        try
        {
            std::regex vertex_regex ( GetVertexBufferRegexPattern ( aMeshBuffer ) );
            while ( std::regex_search ( vertex_string, match_results, vertex_regex ) )
            {
                float float_value;
                uint8_t uint8_t_value;
                size_t index = 1;
                if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_POSITION_BIT )
                {
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                }
                if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_NORMAL_BIT )
                {
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                }
                if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_TANGENT_BIT )
                {
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                }
                if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_BITANGENT_BIT )
                {
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                }
                if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_UV_BIT )
                {
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                    float_value = std::stof ( match_results[index++] );
                    vertex_buffer.append ( reinterpret_cast<char*> ( &float_value ), sizeof ( float ) );
                }
                if ( aMeshBuffer.vertexflags() & MeshBuffer_AttributeBit_WEIGHT_BIT )
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
            return vertex_buffer;
        }
        catch ( std::regex_error& e )
        {
            std::cout << "Error: " << e.what() << " at " << __func__ << " line " << __LINE__ << std::endl;
            std::cout << "Regex: " << GetVertexBufferRegexPattern ( aMeshBuffer ) << std::endl;
            throw;
        }
    }

    std::string ParseIndexBuffer ( const MeshBuffer& aMeshBuffer )
    {
        std::string index_buffer;
        std::smatch match_results;
        std::string index_string{ aMeshBuffer.indexbuffer() };
        try
        {
            std::regex index_regex ( int_pattern );
            while ( std::regex_search ( index_string, match_results, index_regex ) )
            {
                uint8_t uint8_t_value;
                uint16_t uint16_t_value;
                uint32_t uint32_t_value;
                switch ( aMeshBuffer.indextype() )
                {
                case MeshBuffer_IndexTypeEnum_UNSIGNED_BYTE:
                    uint8_t_value = static_cast<uint8_t> ( std::stoi ( match_results[1] ) );
                    index_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                    break;
                case MeshBuffer_IndexTypeEnum_UNSIGNED_SHORT:
                    uint16_t_value = static_cast<uint16_t> ( std::stoi ( match_results[1] ) );
                    index_buffer.append ( reinterpret_cast<char*> ( &uint16_t_value ), sizeof ( uint16_t ) );
                    break;
                case MeshBuffer_IndexTypeEnum_UNSIGNED_INT:
                    uint32_t_value = static_cast<uint32_t> ( std::stoi ( match_results[1] ) );
                    index_buffer.append ( reinterpret_cast<char*> ( &uint32_t_value ), sizeof ( uint32_t ) );
                    break;
                }
                index_string = match_results.suffix();
            }
            return index_buffer;
        }
        catch ( std::regex_error& e )
        {
            std::cout << "Error: " << e.what() << " at " << __func__ << " line " << __LINE__ << std::endl;
            throw;
        }
    }

    Convert::Convert()
        = default;
    Convert::~Convert()
        = default;

    void Convert::ProcessArgs ( int argc, char** argv )
    {
        if ( argc < 2 || ( strcmp ( argv[1], "convert" ) != 0 ) )
        {
            std::ostringstream stream;
            stream << "Invalid tool name, expected convert, got " << ( ( argc < 2 ) ? "nothing" : argv[1] ) << std::endl;
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
            ///@todo Try to guess correct output extension based on the input extension.
            mOutputFile = mInputFile + ".out";
        }
    }
    int Convert::operator() ( int argc, char** argv )
    {
        ProcessArgs ( argc, argv );
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
                        mesh_buffer.set_vertexbuffer ( ParseVertexBuffer ( mesh_buffer ) );
                    }
                    /* Presume raw buffer data if the lenght of the index buffer
                    string exactly maches the string length. */
                    if ( ( mesh_buffer.indexcount() *
                           ( ( mesh_buffer.indextype() == MeshBuffer_IndexTypeEnum_UNSIGNED_BYTE ) ? 1 :
                             ( mesh_buffer.indextype() == MeshBuffer_IndexTypeEnum_UNSIGNED_SHORT ) ? 2 : 4 ) )
                         != mesh_buffer.vertexbuffer().size() )
                    {
                        mesh_buffer.set_indexbuffer ( ParseIndexBuffer ( mesh_buffer ) );
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

                // Print Index buffers in a more human readable format.
                if ( ( message == &mesh_buffer ) && ( !printer.RegisterFieldValuePrinter (
                        mesh_buffer.GetDescriptor()->FindFieldByName ( "IndexBuffer" ),
                        new IndexBufferFieldValuePrinter ( mesh_buffer ) ) ) )
                {
                    std::cout << "Failed to register index buffer printer." << std::endl;
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

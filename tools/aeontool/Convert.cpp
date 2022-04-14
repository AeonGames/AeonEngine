/*
Copyright (C) 2016-2022 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.h"
#include <google/protobuf/text_format.h>
#include "pipeline.pb.h"
#include "material.pb.h"
#include "mesh.pb.h"
#include "skeleton.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <functional>
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
    static const char uint_pattern[] = "(+?[0-9]+)";
    static const char int_pattern[] = "([-+]?[0-9]+)";
    static const char separator_pattern[] = "\\s+";
    uint32_t GetStride ( const MeshMsg& aMeshMsg )
    {
        uint32_t stride = 0;
        for ( const auto& i : aMeshMsg.attribute() )
        {
            switch ( i.type() )
            {
            case AttributeMsg::BYTE:
            case AttributeMsg::UNSIGNED_BYTE:
                stride += i.size();
                break;
            case AttributeMsg::SHORT:
            case AttributeMsg::UNSIGNED_SHORT:
            case AttributeMsg::HALF_FLOAT:
                stride += 2 * i.size();
                break;
            case AttributeMsg::INT:
            case AttributeMsg::UNSIGNED_INT:
            case AttributeMsg::FLOAT:
            case AttributeMsg::FIXED:
                stride += 4 * i.size();
            case AttributeMsg::DOUBLE:
                stride += 8 * i.size();
                break;
            default:
                break;
            }
        }
        return stride;
    }

    std::string GetVertexBufferRegexPattern ( const MeshMsg& aMeshMsg )
    {
        std::string pattern{ "\\(\\s*" };

        for ( const auto& i : aMeshMsg.attribute() )
        {
            switch ( i.type() )
            {
            case AttributeMsg::BYTE:
            case AttributeMsg::SHORT:
            case AttributeMsg::INT:
                for ( size_t j = 0; j < i.size(); ++j )
                {
                    pattern += int_pattern;
                    if ( j < ( i.size() - 1 ) )
                    {
                        pattern += separator_pattern;
                    }
                }
                break;
            case AttributeMsg::UNSIGNED_BYTE:
            case AttributeMsg::UNSIGNED_SHORT:
            case AttributeMsg::UNSIGNED_INT:
                for ( size_t j = 0; j < i.size(); ++j )
                {
                    pattern += uint_pattern;
                    if ( j < ( i.size() - 1 ) )
                    {
                        pattern += separator_pattern;
                    }
                }
                break;
            case AttributeMsg::HALF_FLOAT:
            case AttributeMsg::FLOAT:
            case AttributeMsg::FIXED:
            case AttributeMsg::DOUBLE:
                for ( size_t j = 0; j < i.size(); ++j )
                {
                    pattern += float_pattern;
                    if ( j < ( i.size() - 1 ) )
                    {
                        pattern += separator_pattern;
                    }
                }
                break;
            default:
                break;
            }
        }
        pattern += "\\s*\\)";
        return pattern;
    }

    class CodeFieldValuePrinter : public google::protobuf::TextFormat::FastFieldValuePrinter
    {
    public:
        CodeFieldValuePrinter() : google::protobuf::TextFormat::FastFieldValuePrinter()
        {
        };
        void PrintString ( const std::string & val, google::protobuf::TextFormat::BaseTextGenerator* base_text_generator ) const override
        {
            std::string pattern ( "\\\\n" );
            std::string format ( "$&\"\n\"" );
            try
            {
                std::regex newline ( pattern );
                std::string printed = std::regex_replace ( val, newline, format );
                google::protobuf::TextFormat::FastFieldValuePrinter::PrintString ( printed, base_text_generator );
            }
            catch ( const std::regex_error& e )
            {
                std::cout << "Error: " << e.what() << " at " << __func__ << " line " << __LINE__ << std::endl;
                throw;
            }
        }
    };


    template<class T> const uint8_t* Print ( const uint8_t* cursor, std::ostringstream& stream, uint32_t count )
    {
        const T* value = reinterpret_cast<const T*> ( cursor );
        for ( uint32_t i = 0; i < count; ++i )
        {
            stream << " " << value[i];
        }
        return cursor += sizeof ( T ) * count;
    }

    class VertexBufferFieldValuePrinter : public google::protobuf::TextFormat::FastFieldValuePrinter
    {
    public:
        VertexBufferFieldValuePrinter ( const MeshMsg& aMeshMsg ) :
            google::protobuf::TextFormat::FastFieldValuePrinter(),
            mMeshMsg{ aMeshMsg }
        {
        };
        void PrintBytes ( const std::string & val, google::protobuf::TextFormat::BaseTextGenerator* base_text_generator ) const override
        {
            const uint8_t* cursor = reinterpret_cast<const uint8_t*> ( val.data() );
            std::ostringstream stream;
            stream << std::endl;
            for ( std::size_t i = 0; i < mMeshMsg.vertexcount(); ++i )
            {
                stream << "\"(";
                for ( const auto& i : mMeshMsg.attribute() )
                {
                    switch ( i.type() )
                    {
                    case AttributeMsg::BYTE:
                        cursor = Print<int8_t> ( cursor, stream, i.size() );
                        break;
                    case AttributeMsg::SHORT:
                        cursor = Print<int16_t> ( cursor, stream, i.size() );
                        break;
                    case AttributeMsg::INT:
                        cursor = Print<int32_t> ( cursor, stream, i.size() );
                        break;
                    case AttributeMsg::UNSIGNED_BYTE:
                        cursor = Print<uint8_t> ( cursor, stream, i.size() );
                        break;
                    case AttributeMsg::UNSIGNED_SHORT:
                        cursor = Print<uint16_t> ( cursor, stream, i.size() );
                        break;
                    case AttributeMsg::UNSIGNED_INT:
                        cursor = Print<uint32_t> ( cursor, stream, i.size() );
                        break;
                    case AttributeMsg::HALF_FLOAT:
                        /**todo implement */
                        break;
                    case AttributeMsg::FLOAT:
                        cursor = Print<float> ( cursor, stream, i.size() );
                        break;
                    case AttributeMsg::FIXED:
                        /**todo implement */
                        break;
                    case AttributeMsg::DOUBLE:
                        cursor = Print<double> ( cursor, stream, i.size() );
                        break;
                    default:
                        break;
                    }
                }
                stream << " )\"" << std::endl;
            }
            base_text_generator->PrintString ( stream.str() );
        }
    private:
        const MeshMsg& mMeshMsg;
    };

    class IndexBufferFieldValuePrinter : public google::protobuf::TextFormat::FastFieldValuePrinter
    {
    public:
        IndexBufferFieldValuePrinter ( const MeshMsg& aMeshMsg ) :
            google::protobuf::TextFormat::FastFieldValuePrinter(),
            mMeshMsg{ aMeshMsg }
        {
        };
        void PrintBytes ( const std::string & val, google::protobuf::TextFormat::BaseTextGenerator* base_text_generator ) const override
        {
            const uint8_t* cursor8;
            const uint16_t* cursor16;
            const uint32_t* cursor32 =
                reinterpret_cast<const uint32_t*> ( cursor16 = reinterpret_cast<const uint16_t*> ( cursor8 = reinterpret_cast<const uint8_t*> ( val.data() ) ) );
            std::ostringstream stream;
            stream << std::endl;
            for ( size_t i = 0; i < mMeshMsg.indexcount(); ++i )
            {
                if ( i % 3 == 0 )
                {
                    stream << "\"";
                }
                switch ( mMeshMsg.indexsize() )
                {
                case 1:
                    stream << static_cast<uint32_t> ( cursor8[i] );
                    break;
                case 2:
                    stream << static_cast<uint32_t> ( cursor16[i] );
                    break;
                case 4:
                    stream << cursor32[i];
                    break;
                default:
                    throw std::runtime_error ( "Invalid index size." );
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
            base_text_generator->PrintString ( stream.str() );
        }
    private:
        const MeshMsg& mMeshMsg;
    };


    template<class T> size_t Parse ( size_t index, const std::smatch& match_results, std::string& vertex_buffer, size_t count )
    {
        T value{};
        for ( size_t i = 0; i < count; ++i )
        {
            if constexpr ( std::is_same_v<T, int32_t> )
            {
                value = stoi ( match_results[index++] );
            }
            else if constexpr ( std::is_same_v<T, uint32_t> )
            {
                value = static_cast<T> ( stoul ( match_results[index++] ) );
            }
            else if constexpr ( std::is_same_v<T, float> )
            {
                value = stof ( match_results[index++] );
            }
            else if constexpr ( std::is_same_v<T, double> )
            {
                value = stod ( match_results[index++] );
            }
            vertex_buffer.append ( reinterpret_cast<char*> ( &value ), sizeof ( T ) );
        }
        return index;
    }

    std::string ParseVertexBuffer ( const MeshMsg& aMeshMsg )
    {
        std::string vertex_buffer;
        std::smatch match_results;
        std::string vertex_string{ aMeshMsg.vertexbuffer() };
        try
        {
            std::regex vertex_regex ( GetVertexBufferRegexPattern ( aMeshMsg ) );
            while ( std::regex_search ( vertex_string, match_results, vertex_regex ) )
            {
                size_t index = 1;
                for ( const auto& i : aMeshMsg.attribute() )
                {
                    switch ( i.type() )
                    {
                    case AttributeMsg::BYTE:
                        index = Parse<int8_t> ( index, match_results, vertex_buffer, i.size() );
                        break;
                    case AttributeMsg::SHORT:
                        index = Parse<int16_t> ( index, match_results, vertex_buffer, i.size() );
                        break;
                    case AttributeMsg::INT:
                        index = Parse<int32_t> ( index, match_results, vertex_buffer, i.size() );
                        break;
                    case AttributeMsg::UNSIGNED_BYTE:
                        index = Parse<uint8_t> ( index, match_results, vertex_buffer, i.size() );
                        break;
                    case AttributeMsg::UNSIGNED_SHORT:
                        index = Parse<uint16_t> ( index, match_results, vertex_buffer, i.size() );
                        break;
                    case AttributeMsg::UNSIGNED_INT:
                        index = Parse<uint32_t> ( index, match_results, vertex_buffer, i.size() );
                        break;
                    case AttributeMsg::HALF_FLOAT:
                        /**todo implement */
                        break;
                    case AttributeMsg::FLOAT:
                        index = Parse<float> ( index, match_results, vertex_buffer, i.size() );
                        break;
                    case AttributeMsg::FIXED:
                        /**todo implement */
                        break;
                    case AttributeMsg::DOUBLE:
                        index = Parse<double> ( index, match_results, vertex_buffer, i.size() );
                        break;
                    default:
                        break;
                    }
                }
                vertex_string = match_results.suffix();
            }
            return vertex_buffer;
        }
        catch ( const std::regex_error& e )
        {
            std::cout << "Error: " << e.what() << " at " << __func__ << " line " << __LINE__ << std::endl;
            std::cout << "Regex: " << GetVertexBufferRegexPattern ( aMeshMsg ) << std::endl;
            throw;
        }
    }

    std::string ParseIndexBuffer ( const MeshMsg& aMeshMsg )
    {
        std::string index_buffer;
        std::smatch match_results;
        std::string index_string{ aMeshMsg.indexbuffer() };
        try
        {
            std::regex index_regex ( int_pattern );
            while ( std::regex_search ( index_string, match_results, index_regex ) )
            {
                uint8_t uint8_t_value;
                uint16_t uint16_t_value;
                uint32_t uint32_t_value;
                switch ( aMeshMsg.indexsize() )
                {
                case 1:
                    uint8_t_value = static_cast<uint8_t> ( std::stoi ( match_results[1] ) );
                    index_buffer.append ( reinterpret_cast<char*> ( &uint8_t_value ), sizeof ( uint8_t ) );
                    break;
                case 2:
                    uint16_t_value = static_cast<uint16_t> ( std::stoi ( match_results[1] ) );
                    index_buffer.append ( reinterpret_cast<char*> ( &uint16_t_value ), sizeof ( uint16_t ) );
                    break;
                case 4:
                    uint32_t_value = static_cast<uint32_t> ( std::stoi ( match_results[1] ) );
                    index_buffer.append ( reinterpret_cast<char*> ( &uint32_t_value ), sizeof ( uint32_t ) );
                    break;
                }
                index_string = match_results.suffix();
            }
            return index_buffer;
        }
        catch ( const std::regex_error& e )
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
        PipelineMsg pipeline_buffer;
        MaterialMsg material_buffer;
        MeshMsg mesh_buffer;
        SkeletonMsg skeleton_buffer;
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
            case FileType::AEONPLNB:
                binary_input = true;
            /* coverity[fallthrough] */
            case FileType::AEONPLNT:
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
                    if ( ( mesh_buffer.indexcount() * mesh_buffer.indexsize() ) != mesh_buffer.indexbuffer().size() )
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
            if ( strncmp ( type, "PLN", 3 ) == 0 )
            {
                retval = ( type[3] == '\0' ) ? Convert::FileType::AEONPLNB :
                         Convert::FileType::AEONPLNT;
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

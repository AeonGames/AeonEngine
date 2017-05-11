/*
Copyright (C) 2016-2017 Rodrigo Jose Hernandez Cordoba

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
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <regex>
#ifdef __unix__
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
                std::string text ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
                if ( !google::protobuf::TextFormat::ParseFromString (
                         text,
                         message ) )
                {
                    throw std::runtime_error ( "Text file parsing failed." );
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

/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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


#ifdef __unix__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/text_format.h>
#include "program.pb.h"
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <regex>
#include "Convert.h"

namespace AeonGames
{
    class CodeFieldValuePrinter : public google::protobuf::TextFormat::FieldValuePrinter
    {
    public:
        CodeFieldValuePrinter() : google::protobuf::TextFormat::FieldValuePrinter()
        {
        };
        virtual std::string PrintString ( const std::string & val ) const override
        {
            std::string pattern ( "\\\\n" );
            std::regex newline ( pattern );
            std::string printed ( google::protobuf::TextFormat::FieldValuePrinter::PrintString ( val ) );
            printed = std::regex_replace ( printed, newline, "$&\"\n\"" );
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
    {

    }
    int Convert::Run()
    {
        ProgramBuffer shader_program_buffer;
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
            case FileType::AEONPRGB:
                binary_input = true;
            case FileType::AEONPRGT:
                message = &shader_program_buffer;
                break;
            case FileType::AEONMSHB:
                binary_input = true;
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
#if 1
                // Try to print shader code in a more human readable format.
                if ( ( message == &shader_program_buffer ) && ( !printer.RegisterFieldValuePrinter (
                            shader_program_buffer.vertex_shader().GetDescriptor()->FindFieldByName ( "code" ),
                            new CodeFieldValuePrinter ) ) )
                {
                    std::cout << "Failed to register field value printer." << std::endl;
                }
#endif
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
            else if ( strncmp ( type, "MSH", 3 ) == 0 )
            {
                retval = ( type[3] == '\0' ) ? Convert::FileType::AEONMSHB :
                         Convert::FileType::AEONMSHT;
            }
        }
        return retval;
    }
}

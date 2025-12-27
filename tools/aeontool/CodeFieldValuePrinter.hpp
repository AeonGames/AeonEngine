/*
Copyright (C) 2016-2022,2025 Rodrigo Jose Hernandez Cordoba

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
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include <sstream>
namespace AeonGames
{

    class CodeFieldValuePrinter : public google::protobuf::TextFormat::FastFieldValuePrinter
    {
    public:
        CodeFieldValuePrinter() : google::protobuf::TextFormat::FastFieldValuePrinter()
        {
        };
        void PrintString ( const std::string & val, google::protobuf::TextFormat::BaseTextGenerator* base_text_generator ) const override
        {
            // Format multi-line strings by breaking them after each \n
            // First, escape the string normally (quotes, backslashes, etc.)
            std::string escaped;
            escaped.reserve ( val.size() * 2 );
            for ( char c : val )
            {
                switch ( c )
                {
                case '\\':
                    escaped += "\\\\";
                    break;
                case '\"':
                    escaped += "\\\"";
                    break;
                case '\n':
                    escaped += "\\n";
                    break;
                case '\r':
                    escaped += "\\r";
                    break;
                case '\t':
                    escaped += "\\t";
                    break;
                default:
                    if ( c >= 32 && c < 127 )
                    {
                        escaped += c;
                    }
                    else
                    {
                        // Print as octal escape
                        char buf[5];
                        snprintf ( buf, sizeof ( buf ), "\\%03o", static_cast<unsigned char> ( c ) );
                        escaped += buf;
                    }
                    break;
                }
            }

            // Now split the escaped string at each \n occurrence
            std::ostringstream output;
            output << '"';

            size_t pos = 0;
            size_t found;
            while ( ( found = escaped.find ( "\\n", pos ) ) != std::string::npos )
            {
                // Print up to and including the \n
                output << escaped.substr ( pos, found - pos + 2 );
                // Check if there's more content after this \n
                if ( found + 2 < escaped.size() )
                {
                    output << "\"\n\"";
                }
                pos = found + 2;
            }

            // Print remaining content
            if ( pos < escaped.size() )
            {
                output << escaped.substr ( pos );
            }

            output << '"';
            base_text_generator->PrintString ( output.str() );
        }
    };
}
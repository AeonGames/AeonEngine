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
#include <regex>
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
}
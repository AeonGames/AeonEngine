/*
Copyright (C) 2016,2018,2019 Rodrigo Jose Hernandez Cordoba

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
#ifdef _WIN32
#include "aeongames/AeonEngine.h"
#include "aeongames/Utilities.h"
#include "aeongames/LogLevel.h"
#include <cassert>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <regex>
#include <tuple>
#include <shellapi.h>

/** Convert a WinMain command line (lpCmdLine) into a regular argc,argv pair.
 * @param aCmdLine Windows API WinMain format command line.
 * @return tuple containing a vector of char* (std::get<0>) and a string
 * containing the argument strings separated each by a null character(std::get<1>).
 * @note The string part is meant only as managed storage for the char* vector,
 * as those point into the string memory, so there should not be a real reason
 * to read it directly.
*/
static std::tuple<std::vector<char*>, const std::string> GetArgs ( char* aCmdLine )
{
    /*Match any whitespace OR any whitespace followed by eol OR eol.*/
    std::regex whitespace{"\\s+|\\s+$|$"};
    std::tuple<std::vector<char*>, std::string>
    /*Replace all matches with a single plain old space character.
        Note: using "\0" instead of " " would be great, but "\0"
        gets converted as per the spec into the empty string rather
        than a size 1 string containing the null character.
        And no, "\0\0" does not work, std::string("\0\0",2) does not either.*/
    result{std::vector<char*>{}, std::regex_replace ( aCmdLine, whitespace, " " ) };

    size_t count{0};
    for ( size_t i = 0; i < std::get<1> ( result ).size(); ++i )
    {
        if ( std::get<1> ( result ) [i] == ' ' )
        {
            std::get<1> ( result ) [i] = '\0';
            ++count;
        }
    }

    if ( count )
    {
        std::get<0> ( result ).reserve ( count );
        std::get<0> ( result ).emplace_back ( std::get<1> ( result ).data() );
        for ( uint32_t i = 0; i < std::get<1> ( result ).size() - 1; ++i )
        {
            if ( std::get<1> ( result ) [i] == '\0' )
            {
                std::get<0> ( result ).emplace_back ( std::get<1> ( result ).data() + ( i + 1 ) );
            }
        }
    }
    return result;
}

static void GetArgumentIntoString ( const char* aArgument, void* aUserData )
{
    std::string* string_pointer = reinterpret_cast<std::string*> ( aUserData );
    if ( aArgument && string_pointer )
    {
        *string_pointer = aArgument;
    }
}

extern int Main ( int argc, char *argv[] );

int WINAPI ENTRYPOINT WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    auto args = GetArgs ( lpCmdLine );
    return Main ( std::get<0> ( args ).size(), std::get<0> ( args ).data() );
}

int ENTRYPOINT main ( int argc, char *argv[] )
{
#ifdef _MSC_VER
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#if 0
    // Send all reports to STDOUT
    _CrtSetReportMode ( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile ( _CRT_WARN, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode ( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile ( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode ( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile ( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
#endif
    // Use _CrtSetBreakAlloc( ) to set breakpoints on allocations.
    //_CrtSetBreakAlloc (1159202);
#endif
    std::ostringstream stream;
    for ( uint32_t i = 0; i < argc; ++i )
    {
        stream << argv[i] << ( ( i != argc - 1 ) ? " " : "" );
    }
    std::string command_line = stream.str();
    return WinMain ( GetModuleHandle ( NULL ), NULL, stream.str().data(), 0 );
}
#endif
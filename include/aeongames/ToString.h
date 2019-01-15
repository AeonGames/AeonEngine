/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_TOSTRING_H
#define AEONGAMES_TOSTRING_H

#include <string>
#include <sstream>

namespace AeonGames
{
    /* Taken from https://stackoverflow.com/questions/33399594/making-a-user-defined-class-stdto-stringable */
    namespace to_string_namespace
    {
        using std::to_string;

        template<class T>
        std::string convert_to_string ( T&& t )
        {
            return to_string ( std::forward<T> ( t ) );
        }
    }
    template<class T>
    std::string to_string ( T&& t )
    {
        return to_string_namespace::convert_to_string ( std::forward<T> ( t ) );
    }

    /* Homebrew alternative */
    template<class T>
    std::string ToString ( const T& t )
    {
        std::ostringstream stream;
        const uint8_t* pointer = &t;
        for ( size_t i = 0; i < sizeof ( T ); ++i )
        {
            stream << "0x" << std::hex << pointer[i];
        }
        return stream.str();
    }

    template<> std::string ToString ( const int& t )
    {
        return std::to_string ( t );
    }
    template<> std::string ToString ( const long& t )
    {
        return std::to_string ( t );
    }
    template<> std::string ToString ( const long long& t )
    {
        return std::to_string ( t );
    }
    template<> std::string ToString ( const unsigned& t )
    {
        return std::to_string ( t );
    }
    template<> std::string ToString ( const unsigned long& t )
    {
        return std::to_string ( t );
    }
    template<> std::string ToString ( const unsigned long long& t )
    {
        return std::to_string ( t );
    }
    template<> std::string ToString ( const float& t )
    {
        return std::to_string ( t );
    }
    template<> std::string ToString ( const double& t )
    {
        return std::to_string ( t );
    }
}
#endif

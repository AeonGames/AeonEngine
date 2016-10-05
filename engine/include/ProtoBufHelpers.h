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

#include <fstream>
#include <sstream>
#include <exception>
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/text_format.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Utilities.h"

namespace AeonGames
{
    /** Loads a Protocol Buffer Object from an AeonGames file.
    @param aFilename Path to file to load object from.
    @param aFormat Expected format of the file represented by its magick number.
    @return A fully loaded protocol buffer object.
    @note This version is meant to take advantage of RVO, which should be useful when the object is meant to be persistent.
    */
    template<class T> T LoadProtoBufObject ( const std::string& aFilename, const std::string& aFormat )
    {
        T t;
        if ( !FileExists ( aFilename ) )
        {
            std::ostringstream stream;
            stream << "File " << aFilename << " not found.";
            throw std::runtime_error ( stream.str().c_str() );
        }
        std::ifstream file;
        file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
        file.open ( aFilename, std::ifstream::in | std::ifstream::binary );
        char magick_number[8] = { 0 };
        file.read ( magick_number, sizeof ( magick_number ) );
        file.exceptions ( std::ifstream::badbit );
        if ( strncmp ( magick_number, aFormat.c_str(), 7 ) )
        {
            file.close();
            std::ostringstream stream;
            stream << "File " << aFilename << " Is not an AeonGames format.";
            throw std::runtime_error ( stream.str().c_str() );
        }
        else if ( magick_number[7] == '\0' )
        {
            file.close();
            if ( !t.ParseFromIstream ( &file ) )
            {
                std::ostringstream stream;
                stream << "Binary parse failed on file " << aFilename;
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        else
        {
            file.close();
            std::string text ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
            if ( !google::protobuf::TextFormat::ParseFromString ( text, &t ) )
            {
                std::ostringstream stream;
                stream << "Text parse failed on file " << aFilename;
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        file.close();
        return t;
    }
    /** Loads a Protocol Buffer Object from an AeonGames file into the provided reference.
    @param t Reference to the object to be loaded with the file data.
    @param aFilename Path to file to load object from.
    @param aFormat Expected format of the file represented by its magick number.
    @note ProtoBuf objects require dynamic memory and if those objects get constantly destroyed
    and recreated, the allocations and deallocations may slow down the system so for temporary objects
    it may be better to use a static variable which gets loaded by reference and then explicitly unloaded,
    thus reusing its memory cache.
    However, I have not yet confirmed or denied this as true, the RVO version may be the only one needed.
    */
    template<class T> void LoadProtoBufObject ( T& t, const std::string& aFilename, const std::string& aFormat )
    {
        if ( !FileExists ( aFilename ) )
        {
            std::ostringstream stream;
            stream << "File " << aFilename << " not found.";
            throw std::runtime_error ( stream.str().c_str() );
        }
        std::ifstream file;
        file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
        file.open ( aFilename, std::ifstream::in | std::ifstream::binary );
        char magick_number[8] = { 0 };
        file.read ( magick_number, sizeof ( magick_number ) );
        file.exceptions ( std::ifstream::badbit );
        if ( strncmp ( magick_number, aFormat.c_str(), 7 ) )
        {
            file.close();
            std::ostringstream stream;
            stream << "File " << aFilename << " Is not an AeonGames format.";
            throw std::runtime_error ( stream.str().c_str() );
        }
        else if ( magick_number[7] == '\0' )
        {
            if ( !t.ParseFromIstream ( &file ) )
            {
                std::ostringstream stream;
                stream << "Binary parse failed on file " << aFilename;
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        else
        {
            std::string text ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
            if ( !google::protobuf::TextFormat::ParseFromString ( text, &t ) )
            {
                std::ostringstream stream;
                stream << "Text parse failed on file " << aFilename;
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        file.close();
    }
}

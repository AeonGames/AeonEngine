/*
Copyright (C) 2016,2018,2019,2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PROTOBUFHELPERS_H
#define AEONGAMES_PROTOBUFHELPERS_H
#include <fstream>
#include <sstream>
#include <exception>
#include <limits>
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Utilities.h"

namespace AeonGames
{
    // Helper class to read from a raw memory buffer.
    class BufferInputStream : public google::protobuf::io::ZeroCopyInputStream
    {
    public:
        BufferInputStream ( const void * aData, size_t aSize ) :
            mStart{ reinterpret_cast<const uint8_t*> ( aData ) },
            mCursor{ mStart },
            mEnd{mStart + aSize} {}
        virtual ~BufferInputStream() {}
        bool Next ( const void** data, int* size ) override
        {
            *data = mCursor;
            *size = ( ( mEnd - mCursor ) > std::numeric_limits<int>::max() ) ? std::numeric_limits<int>::max() : static_cast<int> ( mEnd - mCursor );
            mCursor += *size;
            return ( ( mCursor != mEnd ) || *size );
        }
        void BackUp ( int count ) override
        {
            mCursor = ( ( mCursor - count ) > mStart ) ? mCursor - count : mStart;
        }
        bool Skip ( int count ) override
        {
            mCursor = ( ( mCursor + count ) < mEnd ) ? mCursor + count : mEnd;
            return mCursor != mEnd;
        }
        google::protobuf::int64 ByteCount() const override
        {
            return mCursor - mStart;
        }
    private:
        const uint8_t* mStart;
        const uint8_t* mCursor;
        const uint8_t* mEnd;
    };

    /** Loads a Protocol Buffer Object from a meory buffer into the provided reference.
    @param t Reference to the object to be loaded with the file data.
    @param aData Pointer to the start of the buffer.
    @param aSize size of the provided buffer.
    @param aMagick Expected format of the file represented by its magick number.
    @note ProtoBuf objects require dynamic memory and if those objects get constantly destroyed
    and recreated, the allocations and deallocations may slow down the system so for temporary objects
    it may be better to use a static variable which gets loaded by reference and then explicitly unloaded,
    thus reusing its memory cache.
    However, I have not yet confirmed or denied this as true, the RVO version may be the only one needed.
    */
    template<class T> void LoadProtoBufObject ( T& t, const void * aData, size_t aSize, uint64_t aMagick )
    {
        if ( !aData || aSize < 8 )
        {
            throw std::runtime_error ( "Not enough data or null pointer passed to LoadProtoBufObject." );
        }
        if ( strncmp ( reinterpret_cast<const char*> ( aData ), reinterpret_cast<const char*> ( &aMagick ), 7 ) != 0 )
        {
            std::ostringstream stream;
            stream << "Provided buffer does not contain " << reinterpret_cast<const char*> ( &aMagick ) << " information.";
            throw std::runtime_error ( stream.str().c_str() );
        }
        else if ( reinterpret_cast<const uint8_t*> ( aData ) [7] == '\0' )
        {
            BufferInputStream buffer_input_stream ( reinterpret_cast<const uint8_t*> ( aData ) + 8, aSize - 8 );
            if ( !t.ParseFromZeroCopyStream ( &buffer_input_stream ) )
            {
                std::ostringstream stream;
                stream << "Binary parse failed on " << reinterpret_cast<const char*> ( &aMagick ) << "buffer.";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        else
        {
            BufferInputStream buffer_input_stream ( reinterpret_cast<const uint8_t*> ( aData ) + 8, aSize - 8 );
            if ( !google::protobuf::TextFormat::Parse ( &buffer_input_stream, &t ) )
            {
                std::ostringstream stream;
                stream << "Text parse failed on " << reinterpret_cast<const char*> ( &aMagick ) << " buffer.";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
    }

    /** Loads a Protocol Buffer Object from a memory buffer.
    @param aData Pointer to the start of the buffer.
    @param aSize size of the provided buffer.
    @param aMagick Expected format of the file represented by its magick number.
    @return A fully loaded protocol buffer object.
    @note This version is meant to take advantage of RVO, which should be useful when the object is meant to be persistent.
    */
    template<class T> T LoadProtoBufObject ( const void * aData, size_t aSize, uint64_t aMagick )
    {
        T t;
        LoadProtoBufObject ( t, aData, aSize, aMagick );
        return t;
    }

    /** Loads a Protocol Buffer Object from an AeonGames file into the provided reference.
    @param t Reference to the object to be loaded with the file data.
    @param aFilename Path to file to load object from.
    @param aMagick Expected format of the file represented by its magick number.
    @note ProtoBuf objects require dynamic memory and if those objects get constantly destroyed
    and recreated, the allocations and deallocations may slow down the system so for temporary objects
    it may be better to use a static variable which gets loaded by reference and then explicitly unloaded,
    thus reusing its memory cache.
    However, I have not yet confirmed or denied this as true, the RVO version may be the only one needed.
    */
    template<class T> void LoadProtoBufObject ( T& t, const std::string& aFilename, uint64_t aMagick )
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
        file.exceptions ( std::ifstream::badbit );
        std::string text ( ( std::istreambuf_iterator<char> ( file ) ), std::istreambuf_iterator<char>() );
        file.close();
        LoadProtoBufObject ( t, text.data(), text.size(), aMagick );
    }

    /** Loads a Protocol Buffer Object from an AeonGames file.
    @param aFilename Path to file to load object from.
    @param aMagick Expected format of the file represented by its magick number.
    @return A fully loaded protocol buffer object.
    @note This version is meant to take advantage of RVO, which should be useful when the object is meant to be persistent.
    */
    template<class T> T LoadProtoBufObject ( const std::string& aFilename, uint64_t aMagick )
    {
        T t;
        LoadProtoBufObject ( t, aFilename, aMagick );
        return t;
    }
}
#endif

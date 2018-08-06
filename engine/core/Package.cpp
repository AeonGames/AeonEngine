/*
Copyright (C) 2013,2018 Rodrigo Jose Hernandez Cordoba

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

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <regex>
#include "zlib.h"
#include "aeongames/Package.h"
#include "aeongames/CRC.h"

namespace AeonGames
{
#if 0
#define CHUNK 16384
    /* Decompress from file source to memory dest until stream ends or EOF.
       read_inflated_data() returns Z_OK on success, Z_MEM_ERROR if memory
       could not be allocated for processing, Z_DATA_ERROR if the deflate data is
       invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
       the version of the library linked do not match, or Z_ERRNO if there
       is an error reading or writing the files. */
    static int read_inflated_data ( FILE* source, uint8_t* buffer, uint32_t buffer_size )
    {
        int ret;
        z_stream strm;
        unsigned char in[CHUNK];

        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        ret = inflateInit ( &strm );
        if ( ret != Z_OK )
        {
            return ret;
        }
        strm.avail_out = buffer_size;
        strm.next_out = buffer;

        /* decompress until deflate stream ends or end of file */
        do
        {
            strm.avail_in = static_cast<uInt> ( fread ( in, 1, CHUNK, source ) );
            if ( ferror ( source ) )
            {
                ( void ) inflateEnd ( &strm );
                return Z_ERRNO;
            }
            if ( strm.avail_in == 0 )
            {
                break;
            }
            strm.next_in = in;
            ret = inflate ( &strm, Z_NO_FLUSH );
            assert ( ret != Z_STREAM_ERROR ); /* state not clobbered */
            switch ( ret )
            {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                ( void ) inflateEnd ( &strm );
                return ret;
            }
            /* done when inflate() says it's done */
        }
        while ( ret != Z_STREAM_END );
        /* clean up and return */
        ( void ) inflateEnd ( &strm );
        return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
    }
#endif
    Package::Package ( const std::string& aPath ) : mPath {aPath}, mIndexTable {}
    {
        if ( std::filesystem::is_directory ( mPath ) )
        {
            static const std::string format ( "/" );
            static const std::regex separator ( "\\\\+" );
            for ( auto& i : std::filesystem::recursive_directory_iterator ( mPath ) )
            {
                if ( std::filesystem::is_regular_file ( i ) )
                {
                    std::string location{std::regex_replace ( std::filesystem::relative ( i, mPath ).string(), separator, format ) };
                    mIndexTable[crc32i ( location.data(), location.size() )] = location;
                }
            }
        }
    }

    Package::~Package()
    {
    }
    const std::unordered_map<uint32_t, std::string>& Package::GetIndexTable() const
    {
        return mIndexTable;
    }
    size_t Package::GetFileSize ( uint32_t crc ) const
    {
        if ( std::filesystem::is_directory ( mPath ) )
        {
            auto it = mIndexTable.find ( crc );
            if ( it != mIndexTable.end() )
            {
                return std::filesystem::file_size ( mPath / it->second );
            }
        }
        return 0;
    }
    size_t Package::GetFileSize ( const std::string& aFilename ) const
    {
        return GetFileSize ( crc32i ( aFilename.data(), aFilename.size() ) );
    }
    void Package::LoadFile ( const std::string& aFilename, void* buffer, size_t buffer_size ) const
    {
        LoadFile ( crc32i ( aFilename.data(), aFilename.size() ), buffer, buffer_size );
    }
    void Package::LoadFile ( uint32_t crc, void* buffer, size_t buffer_size ) const
    {
        if ( std::filesystem::is_directory ( mPath ) )
        {
            auto it = mIndexTable.find ( crc );
            if ( it != mIndexTable.end() )
            {
                std::ifstream file;
                file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
                file.open ( mPath / it->second, std::ifstream::in | std::ifstream::binary );
                file.read ( reinterpret_cast<char*> ( buffer ), buffer_size );
                file.close();
            }
        }
    }
}

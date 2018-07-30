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
#include <iostream>
#include <stdexcept>
#include <sstream>
#include "zlib.h"
#include "aeongames/Package.h"
#include "aeongames/CRC.h"

namespace AeonGames
{
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
    static int CompareDirectoryEntries ( const void * A, const void * B )
    {
        const uint32_t a = *reinterpret_cast<const uint32_t*> ( A );
        const PKGDirectoryEntry* b = reinterpret_cast<const PKGDirectoryEntry*> ( B );
        if ( a > b->path )
        {
            return 1;
        }
        else if ( a < b->path )
        {
            return -1;
        }
        return 0;
    }

    Package::Package ( const std::string& aPath ) : mPath{aPath}, mHeader{}, mHandle{}, mDirectory{}, mStringTable{}
    {
        struct stat path_stats {};
        if ( ::stat ( mPath.c_str(), &path_stats ) != 0 )
        {
            std::ostringstream stream;
            stream << "Path not found: ( " << mPath << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        if ( S_ISDIR ( path_stats.st_mode ) )
        {
            std::cout << mPath << " is a directory" << std::endl;
        }
    }

    Package::~Package()
    {
        Close();
    }

    void Package::Open()
    {
        if ( mHandle )
        {
            throw std::runtime_error ( "File already open" );
        }
        if ( ( mHandle = fopen ( mPath.c_str(), "rb" ) ) == nullptr )
        {
            std::ostringstream stream;
            stream << "Could not open file: ( " << mPath << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        fread ( &mHeader, sizeof ( PKGHeader ), 1, mHandle );
        if ( strncmp ( mHeader.id, "AEONPKG\0", 8 ) != 0 )
        {
            Close();
        }
        mDirectory.resize ( mHeader.file_count );
        for ( uint32_t i = 0; i < mHeader.file_count; ++i )
        {
            fread ( &mDirectory[i], sizeof ( PKGDirectoryEntry ), 1, mHandle );
        }
        mStringTable.resize ( mHeader.file_size - mHeader.string_table_offset );
        fseek ( mHandle, mHeader.string_table_offset, SEEK_SET );
        fread ( mStringTable.data(), sizeof ( uint8_t ), mHeader.file_size - mHeader.string_table_offset, mHandle );
    }

    void Package::Close()
    {
        if ( mHandle != nullptr )
        {
            fclose ( mHandle );
            mHandle = nullptr;
        }
        mDirectory.clear();
        mStringTable.clear();
        memset ( &mHeader, 0, sizeof ( PKGHeader ) );
    }

    uint32_t Package::GetFileCount() const
    {
        return mHeader.file_count;
    }

    uint32_t Package::GetFileSize ( uint32_t crc ) const
    {
        PKGDirectoryEntry* directory_entry =
            reinterpret_cast<PKGDirectoryEntry*> (
                bsearch ( &crc,
                          mDirectory.data(),
                          mHeader.file_count,
                          sizeof ( PKGDirectoryEntry ),
                          CompareDirectoryEntries ) );
        if ( directory_entry != nullptr )
        {
            return directory_entry->uncompressed_size;
        }
        return 0;
    }

    const PKGDirectoryEntry* Package::GetFileByIndex ( uint32_t index ) const
    {
        return ( index < mHeader.file_count ) ? &mDirectory[index] : nullptr;
    }

    struct StringTableEntry
    {
        uint32_t CRC;     /// String CRC
        uint32_t offset;  /// String Offset
    };

    static int CompareCRCs ( const void * A, const void * B )
    {
        const uint32_t a = *reinterpret_cast<const uint32_t*> ( A );
        const StringTableEntry* b = reinterpret_cast<const StringTableEntry*> ( B );
        if ( a > b->CRC )
        {
            return 1;
        }
        else if ( a < b->CRC )
        {
            return -1;
        }
        return 0;
    }

    const char* Package::GetFilePath ( uint32_t crc ) const
    {
        StringTableEntry* entry =
            reinterpret_cast<StringTableEntry*> (
                bsearch ( &crc,
                          mStringTable.data() + sizeof ( uint32_t ),
                          *reinterpret_cast<const uint32_t*> ( mStringTable.data() ),
                          sizeof ( StringTableEntry ),
                          CompareCRCs ) );
        if ( entry != NULL )
        {
            return reinterpret_cast<const char*> ( mStringTable.data() ) + entry->offset;
        }
        return nullptr;
    }

    const PKGDirectoryEntry* Package::ContainsFile ( uint32_t crc ) const
    {
        PKGDirectoryEntry* directory_entry =
            reinterpret_cast<PKGDirectoryEntry*> (
                bsearch ( &crc,
                          mDirectory.data(),
                          mHeader.file_count,
                          sizeof ( PKGDirectoryEntry ),
                          CompareDirectoryEntries ) );
        return directory_entry;
    }

    void Package::LoadFile ( uint32_t crc, uint8_t* buffer, uint32_t buffer_size ) const
    {
        PKGDirectoryEntry* directory_entry =
            reinterpret_cast<PKGDirectoryEntry*> (
                bsearch ( &crc,
                          mDirectory.data(),
                          mHeader.file_count,
                          sizeof ( PKGDirectoryEntry ),
                          CompareDirectoryEntries ) );
        if ( directory_entry == nullptr )
        {
            throw std::runtime_error ( "File not found." );
        }
        if ( directory_entry->uncompressed_size < buffer_size )
        {
            throw std::runtime_error ( "Insuficient buffer size." );
        }
        if ( fseek ( mHandle, directory_entry->offset, SEEK_SET ) != 0 )
        {
            std::ostringstream stream;
            stream << "fseek failed with errno: ( " << errno << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        switch ( directory_entry->compression_type )
        {
        case NONE:
            if ( fread ( buffer, buffer_size, 1, mHandle ) == 0 )
            {
                std::ostringstream stream;
                stream << "fread failed with errno: ( " << errno << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
            break;
        case ZLIB:
            if ( int result = read_inflated_data ( mHandle, buffer, buffer_size ) )
            {
                std::ostringstream stream;
                stream << "read_inflated_data failed with error: ( " << result << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
            break;
        default:
        {
            std::ostringstream stream;
            stream << "Unrecognized compression type, or type not supported: ( " << directory_entry->compression_type << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        }
    }
}

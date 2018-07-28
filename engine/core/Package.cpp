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

    Package::Package() : mHeader{}, mHandle{}, mDirectory{}, mStringTable{}
    {
    }

    Package::~Package()
    {
        Close();
    }

    bool Package::Open ( const char* filename )
    {
        Close();
        if ( ( mHandle = fopen ( filename, "rb" ) ) == nullptr )
        {
            //AEONGAMES_LOG_ERROR ( "Could not open file %s", filename );
            return false;
        }
        fread ( &mHeader, sizeof ( PKGHeader ), 1, mHandle );
        if ( strncmp ( mHeader.id, "AEONPKG\0", 8 ) != 0 )
        {
            Close();
            return false;
        }
        mDirectory.resize ( mHeader.file_count );
        for ( uint32_t i = 0; i < mHeader.file_count; ++i )
        {
            fread ( &mDirectory[i], sizeof ( PKGDirectoryEntry ), 1, mHandle );
        }
        mStringTable.resize ( mHeader.file_size - mHeader.string_table_offset );
        fseek ( mHandle, mHeader.string_table_offset, SEEK_SET );
        fread ( mStringTable.data(), sizeof ( uint8_t ), mHeader.file_size - mHeader.string_table_offset, mHandle );
        return true;
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

    bool Package::LoadFile ( uint32_t crc, uint8_t* buffer, uint32_t buffer_size ) const
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
            return false;
        }
        if ( directory_entry->uncompressed_size < buffer_size )
        {
            //AEONGAMES_LOG_ERROR ( "Insuficient Buffer Size in %s line %i.", __FUNCTION__, __LINE__ );
            return false;
        }
        if ( !fseek ( mHandle, directory_entry->offset, SEEK_SET ) )
        {
            return false;
        }
        switch ( directory_entry->compression_type )
        {
        case NONE:
            if ( fread ( buffer, buffer_size, 1, mHandle ) == 0 )
            {
                //AEONGAMES_LOG_ERROR ( "fread Failed in %s line %i.", __FUNCTION__, __LINE__ );
                return false;
            }
            break;
        case ZLIB:
            if ( read_inflated_data ( mHandle, buffer, buffer_size ) != Z_OK )
            {
                //AEONGAMES_LOG_ERROR ( "fread Failed in %s line %i.", __FUNCTION__, __LINE__ );
                return false;
            }
            break;
        default:
            //AEONGAMES_LOG_ERROR ( "Unrecognized compression type, or type not supported: %i", directory_entry->compression_type );
            return false;
        }
        return true;
    }
}

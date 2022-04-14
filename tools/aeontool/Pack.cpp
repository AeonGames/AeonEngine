/*
Copyright (C) 2013,2018,2019,2022 Rodrigo Jose Hernandez Cordoba

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
#ifdef WIN32
#include <windows.h>
#endif
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cassert>
#include "Pack.h"
#include "aeongames/Package.h"
#include "aeongames/CRC.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "zlib.h"
#ifndef WIN32
#include <libgen.h>
#include <dirent.h>
#endif
#include <cstring>

namespace AeonGames
{
#define CHUNK 16384
    /* Compress from file source to file dest until EOF on source.
       def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
       allocated for processing, Z_STREAM_ERROR if an invalid compression
       level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
       version of the library linked do not match, or Z_ERRNO if there is
       an error reading or writing the files.
       compressed_size contains the size in bytes of the compressed written file */
    int write_deflated_data ( FILE *source, FILE *dest, int level, uint32_t& compressed_size )
    {
        int ret, flush;
        unsigned have;
        z_stream strm;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];
        compressed_size = 0;
        /* allocate deflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        ret = deflateInit ( &strm, level );
        if ( ret != Z_OK )
        {
            return ret;
        }

        /* compress until end of file */
        do
        {
            strm.avail_in = static_cast<uInt> ( fread ( in, 1, CHUNK, source ) );
            if ( ferror ( source ) )
            {
                ( void ) deflateEnd ( &strm );
                return Z_ERRNO;
            }
            flush = feof ( source ) ? Z_FINISH : Z_NO_FLUSH;
            strm.next_in = in;

            /* run deflate() on input until output buffer not full, finish
               compression if all of source has been read in */
            do
            {
                strm.avail_out = CHUNK;
                strm.next_out = out;
                ret = deflate ( &strm, flush ); /* no bad return value */
                assert ( ret != Z_STREAM_ERROR ); /* state not clobbered */
                have = CHUNK - strm.avail_out;
                if ( fwrite ( out, 1, have, dest ) != have || ferror ( dest ) )
                {
                    ( void ) deflateEnd ( &strm );
                    return Z_ERRNO;
                }
                compressed_size += have;
            }
            while ( strm.avail_out == 0 );
            assert ( strm.avail_in == 0 );  /* all input will be used */

            /* done when last data in file processed */
        }
        while ( flush != Z_FINISH );
        assert ( ret == Z_STREAM_END );     /* stream will be complete */

        /* clean up and return */
        ( void ) deflateEnd ( &strm );
        return Z_OK;
    }
    bool Pack::ProcessDirectory ( const std::string& path )
    {
#ifdef WIN32
        std::string wildcard = mInputPath + path + "*";
        WIN32_FIND_DATA wfd;
        HANDLE hFind = FindFirstFile ( wildcard.c_str(), &wfd );
        if ( INVALID_HANDLE_VALUE != hFind )
        {
            do
            {
                if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {
                    if ( ( strncmp ( wfd.cFileName, ".\0", 2 ) != 0 ) && ( strncmp ( wfd.cFileName, "..\0", 3 ) != 0 ) )
                    {
                        ProcessDirectory ( path + wfd.cFileName + "/" );
                    }
                }
                else
                {
                    std::string filepath = path + wfd.cFileName;
                    uint32_t filepathcrc = crc32i ( filepath.c_str(), filepath.size() );
                    mStringTable[filepathcrc] = filepath;
                    mDirectory[filepathcrc].path = filepathcrc;
                    mDirectory[filepathcrc].uncompressed_size = wfd.nFileSizeLow; //(wfd.nFileSizeHigh * (MAXDWORD+1)) + wfd.nFileSizeLow;
                    mDirectory[filepathcrc].extension_offset = static_cast<uint32_t> ( filepath.rfind ( '.' ) );
                    if ( mDirectory[filepathcrc].extension_offset != std::string::npos )
                    {
                        ++mDirectory[filepathcrc].extension_offset;
                    }
                    else
                    {
                        mDirectory[filepathcrc].extension_offset = 0;
                    }
                }
            }
            while ( FindNextFile ( hFind, &wfd ) );
            FindClose ( hFind );
        }
#else
        std::string wildcard = mInputPath + path;
        struct dirent* directory_entry;
        DIR* dir;
        if ( ( dir = opendir ( wildcard.c_str() ) ) == NULL )
        {
            return false;
        }
        while ( ( directory_entry = readdir ( dir ) ) != NULL )
        {
            struct stat file_stat;
            std::string fullpath = wildcard + directory_entry->d_name;
            if ( stat ( fullpath.c_str(), &file_stat ) == 0 )
            {
                if ( S_ISDIR ( file_stat.st_mode ) )
                {
                    if ( ( strncmp ( directory_entry->d_name, ".\0", 2 ) != 0 ) && ( strncmp ( directory_entry->d_name, "..\0", 3 ) != 0 ) )
                    {
                        ProcessDirectory ( path + directory_entry->d_name + "/" );
                    }
                }
                else if ( S_ISREG ( file_stat.st_mode ) )
                {
                    std::string filepath = path + directory_entry->d_name;
                    uint32_t filepathcrc = crc32i ( filepath.c_str(), filepath.length() );
                    mStringTable[filepathcrc] = filepath;
                    mDirectory[filepathcrc].path = filepathcrc;
                    mDirectory[filepathcrc].uncompressed_size = file_stat.st_size;
                    mDirectory[filepathcrc].compressed_size = mDirectory[filepathcrc].uncompressed_size;
                    mDirectory[filepathcrc].extension_offset = filepath.rfind ( '.' );
                    if ( mDirectory[filepathcrc].extension_offset != std::string::npos )
                    {
                        ++mDirectory[filepathcrc].extension_offset;
                    }
                    else
                    {
                        mDirectory[filepathcrc].extension_offset = 0;
                    }
                }
            }
        }
        closedir ( dir );
#endif
        return true;
    }

    Pack::Pack()
        = default;
    Pack::~Pack()
        = default;
    void Pack::ProcessArgs ( int argc, char** argv )
    {
        if ( argc < 2 || ( strcmp ( argv[1], "pack" ) != 0 ) )
        {
            std::ostringstream stream;
            stream << "Invalid tool name, expected pack, got " << ( ( argc < 2 ) ? "nothing" : argv[1] ) << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        for ( int i = 2; i < argc; ++i )
        {
            if ( argv[i][0] == '-' )
            {
                if ( argv[i][1] == '-' )
                {
                    if ( strncmp ( &argv[i][2], "in", sizeof ( "in" ) ) == 0 )
                    {
                        i++;
                        mInputPath = argv[i];
                    }
                    else if ( strncmp ( &argv[i][2], "out", sizeof ( "out" ) ) == 0 )
                    {
                        i++;
                        mOutputFile = argv[i];
                    }
                    else if ( strncmp ( &argv[i][2], "extract\0", 8 ) == 0 )
                    {
                        mAction = Action::Extract;
                    }
                    else if ( strncmp ( &argv[i][2], "compress\0", 9 ) == 0 )
                    {
                        mAction = Action::Compress;
                    }
                    else if ( strncmp ( &argv[i][2], "directory\0", 10 ) == 0 )
                    {
                        mAction = Action::Directory;
                    }
                }
                else
                {
                    switch ( argv[i][1] )
                    {
                    case 'i':
                        i++;
                        mInputPath = argv[i];
                        break;
                    case 'o':
                        i++;
                        mOutputFile = argv[i];
                        break;
                    case 'c':
                        mAction = Action::Compress;
                        break;
                    case 'e':
                        mAction = Action::Extract;
                        break;
                    case 'd':
                        mAction = Action::Directory;
                        break;
                    }
                }
            }
            else
            {
                mInputPath = argv[i];
            }
        }
        if ( mInputPath.empty() )
        {
            throw std::runtime_error ( "No Input path provided." );
        }
    }
    int Pack::operator() ( int argc, char** argv )
    {
        mStringTable.clear();
        mDirectory.clear();
        ProcessArgs ( argc, argv );
        switch ( mAction )
        {
        case Action::Compress:
            return ExecCompress();
        case Action::Extract:
            return ExecExtract();
        case Action::Directory:
            return ExecDirectory();
        default:
            break;
        }
        return -1;
    }
    int Pack::ExecExtract() const
    {
        return 0;
    }
    int Pack::ExecDirectory() const
    {
        try
        {
            Package package{mInputPath};
            for ( auto& i : package.GetIndexTable() )
            {
                std::cout << std::setfill ( '0' ) << std::setw ( 10 ) << /*std::hex <<*/ i.first << "\t" << i.second << std::endl;
            }
        }
        catch ( const std::runtime_error& e )
        {
            std::cout << e.what();
            return -1;
        }
        return 0;
    }
    int Pack::ExecCompress() const
    {
#if 0
// Commented out, this old file format might change in the future.
#ifdef _WIN32
        DWORD file_attributes = GetFileAttributes ( mInputPath.c_str() );
        char drive[_MAX_DRIVE];
        char   dir[_MAX_DIR];
        char fname[_MAX_FNAME];
        _splitpath ( mInputPath.c_str(), drive, dir, fname, NULL );
        if ( fname[0] == 0 )
        {
            dir[strlen ( dir ) - 1] = 0;
        }
        if ( mOutputFile.empty() )
        {
            mOutputFile = drive;
            mOutputFile += dir;
            mOutputFile += fname;
            mOutputFile += ".pkg";
        }

        mRootPath = drive;
        mRootPath += dir;
        mRootPath += fname;
        mBaseName = mRootPath.substr ( mRootPath.find_last_of ( '/' ) + 1 );
        mRootPath.erase ( mRootPath.find_last_of ( '/' ) + 1 );
        if ( file_attributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            std::cout << mInputPath << " is a directory" << std::endl;
            if ( mInputPath[mInputPath.length() - 1] != '/' )
            {
                mInputPath += "/";
            }
            if ( !ProcessDirectory ( "" ) )
            {
                return -1;
            }
        }
        else if ( file_attributes != INVALID_FILE_ATTRIBUTES )
        {
            std::cout << mInputPath << " is a file" << std::endl;
        }
        else
        {
            std::cout << mInputPath << " is not a file or a directory" << std::endl;
            return -1;
        }
#else
        struct stat file_stat;
        char path_buffer[1024];
        char file_buffer[1024];
        char* path;
        char* file;
        sprintf ( path_buffer, "%s", mInputPath.c_str() );
        sprintf ( file_buffer, "%s", mInputPath.c_str() );
        path = dirname ( path_buffer );
        file = basename ( file_buffer );
        if ( mOutputFile.empty() )
        {
            mOutputFile = path;
            mOutputFile += "/";
            mOutputFile += file;
            mOutputFile += ".pkg";
        }
        mRootPath = path;
        mRootPath += "/";
        mRootPath += file;
        if ( stat ( mInputPath.c_str(), &file_stat ) != 0 )
        {
            return -1;
        }
        if ( S_ISDIR ( file_stat.st_mode ) )
        {
            std::cout << mInputPath << " is a directory" << std::endl;
            if ( mInputPath[mInputPath.length() - 1] != '/' )
            {
                mInputPath += "/";
            }
            if ( !ProcessDirectory ( "" ) )
            {
                return -1;
            }
        }
#endif
        std::cout << "Packing into: " << mOutputFile << std::endl;
        PKGHeader header;
        header.id[0] = 'A';
        header.id[1] = 'E';
        header.id[2] = 'O';
        header.id[3] = 'N';
        header.id[4] = 'P';
        header.id[5] = 'K';
        header.id[6] = 'G';
        header.id[7] = 0;
        header.version[0] = 0;
        header.version[1] = 1;
        header.file_count = static_cast<uint32_t> ( mDirectory.size() );
        FILE* out = fopen ( mOutputFile.c_str(), "wb" );
        if ( out == NULL )
        {
            return -1;
        }
        fseek ( out, sizeof ( PKGHeader ) + ( sizeof ( PKGDirectoryEntry ) *header.file_count ), SEEK_SET );
        for ( std::unordered_map<uint32_t, PKGDirectoryEntry>::iterator i = mDirectory.begin(); i != mDirectory.end(); ++i )
        {
            size_t bytes_read;
            uint8_t buffer[1024];
            std::string fullpath = mInputPath + mStringTable[i->first];
            std::cout << std::setfill ( '0' ) << std::setw ( 8 ) << std::hex << i->first << " " << fullpath << std::endl;

            i->second.compressed_size = 0;
            FILE* in = fopen ( fullpath.c_str(), "rb" );
            if ( in == NULL )
            {
                std::cout << "WARNING, could not read " << fullpath << std::endl;
                continue;
            }
            i->second.offset = ftell ( out );
            std::string extension;
            size_t ext_idx = fullpath.rfind ( "." );
            if ( ext_idx != std::string::npos )
            {
                extension = fullpath.substr ( ext_idx + 1 );
                std::transform ( extension.begin(), extension.end(), extension.begin(), ::tolower );
            }
            else
            {
                extension.clear();
            }
            if ( ( mAction == Action::Compress ) && ( extension != "png" ) ) /* don't compress PNG files which are already compressed */
            {
                i->second.compression_type = 1;
                if ( write_deflated_data ( in, out, 5, i->second.compressed_size ) != Z_OK )
                {
                    std::cout << "Deflate Failed on" << fullpath << std::endl;
                }
            }
            else
            {
                i->second.compression_type = 0;
                while ( ( bytes_read = fread ( buffer, sizeof ( uint8_t ), 1024, in ) ) != 0 )
                {
                    i->second.compressed_size += static_cast<uint32_t> ( fwrite ( buffer, sizeof ( uint8_t ), bytes_read, out ) );
                }
                assert ( ( i->second.compression_type == 0 ) && ( i->second.uncompressed_size == i->second.compressed_size ) );
            }
            fclose ( in );
        }

        // Write String table------------------------------------------------------------------------------------------------
        header.string_table_offset = ftell ( out );
        uint32_t string_table_size = static_cast<uint32_t> ( mStringTable.size() );
        fwrite ( &string_table_size, sizeof ( uint32_t ), 1, out );
        fseek ( out, sizeof ( uint32_t ) * 2 * string_table_size, SEEK_CUR );
        std::unordered_map<uint32_t, uint32_t> string_table_offsets;
        for ( std::unordered_map<uint32_t, std::string>::iterator i = mStringTable.begin(); i != mStringTable.end(); ++i )
        {
            string_table_offsets[i->first] = ftell ( out ) - header.string_table_offset;
            fwrite ( i->second.c_str(), i->second.size(), 1, out );
            fwrite ( "\0", 1, 1, out );
        }
        fseek ( out, header.string_table_offset + sizeof ( uint32_t ), SEEK_SET );
        for ( std::unordered_map<uint32_t, uint32_t>::iterator i = string_table_offsets.begin(); i != string_table_offsets.end(); ++i )
        {
            fwrite ( &i->first, sizeof ( uint32_t ), 1, out );
            fwrite ( &i->second, sizeof ( uint32_t ), 1, out );
        }
        //-------------------------------------------------------------------------------------------------------------------
        fseek ( out, 0, SEEK_END );
        header.file_size = static_cast<uint32_t> ( ftell ( out ) );
        fseek ( out, 0, SEEK_SET );
        fwrite ( &header, sizeof ( PKGHeader ), 1, out );
        for ( std::unordered_map<uint32_t, PKGDirectoryEntry>::iterator i = mDirectory.begin(); i != mDirectory.end(); ++i )
        {
            fwrite ( &i->second, sizeof ( PKGDirectoryEntry ), 1, out );
        }
        fclose ( out );
#endif
        return 0;
    }
}

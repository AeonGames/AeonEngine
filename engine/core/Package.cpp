/*
Copyright (C) 2013,2018,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <regex>
#include <algorithm>
#include <system_error>
#include <vector>
#include "zlib.h"
#include "aeongames/Package.hpp"
#include "aeongames/CRC.hpp"

namespace AeonGames
{
    namespace
    {
        constexpr size_t kInflateChunk = 16384;

        /** Decompress exactly `compressed_size` bytes starting at `offset` of
            `file` into `buffer` (capacity `buffer_size`). Reads in chunks so we
            don't have to load the whole compressed payload at once.
            Returns Z_OK on success, a zlib error code otherwise. */
        int InflateRegion ( std::ifstream& file,
                            uint64_t offset,
                            uint64_t compressed_size,
                            void* buffer,
                            size_t buffer_size )
        {
            file.clear();
            file.seekg ( static_cast<std::streamoff> ( offset ), std::ios::beg );
            if ( !file )
            {
                return Z_ERRNO;
            }

            z_stream strm{};
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            int ret = inflateInit ( &strm );
            if ( ret != Z_OK )
            {
                return ret;
            }
            strm.next_out = static_cast<Bytef*> ( buffer );
            strm.avail_out = static_cast<uInt> ( buffer_size );

            unsigned char in[kInflateChunk];
            uint64_t remaining = compressed_size;
            while ( remaining > 0 )
            {
                const std::streamsize want =
                    static_cast<std::streamsize> ( std::min<uint64_t> ( remaining, kInflateChunk ) );
                file.read ( reinterpret_cast<char*> ( in ), want );
                const std::streamsize got = file.gcount();
                if ( got <= 0 )
                {
                    inflateEnd ( &strm );
                    return Z_ERRNO;
                }
                strm.next_in = in;
                strm.avail_in = static_cast<uInt> ( got );
                remaining -= static_cast<uint64_t> ( got );

                while ( strm.avail_in > 0 )
                {
                    ret = inflate ( &strm, ( remaining == 0 ) ? Z_FINISH : Z_NO_FLUSH );
                    if ( ret == Z_STREAM_END )
                    {
                        break;
                    }
                    if ( ret != Z_OK )
                    {
                        inflateEnd ( &strm );
                        return ret;
                    }
                    if ( strm.avail_out == 0 && strm.avail_in > 0 )
                    {
                        // Caller buffer is full but more compressed input remains.
                        inflateEnd ( &strm );
                        return Z_BUF_ERROR;
                    }
                }
                if ( ret == Z_STREAM_END )
                {
                    break;
                }
            }
            inflateEnd ( &strm );
            return ( ret == Z_STREAM_END || ret == Z_OK ) ? Z_OK : Z_DATA_ERROR;
        }

        /** On-disk header for AEONIDX index files written by aeontool index.
            See tools/aeontool/Index.cpp. */
        struct IDXHeader
        {
            char id[8];          /**< "AEONIDX\0" */
            uint16_t version[2]; /**< [0]=major, [1]=minor */
            uint32_t count;      /**< Number of entries */
        };
        struct IDXEntry
        {
            uint32_t crc;
            uint32_t path_offset; /**< Byte offset into string blob */
        };
        static_assert ( sizeof ( IDXHeader ) == 16, "IDXHeader must be 16 bytes" );
        static_assert ( sizeof ( IDXEntry ) == 8, "IDXEntry must be 8 bytes" );

        /** Try to populate aIndexTable from an AEONIDX file at aIndexPath.
            @return true on success, false if the file is missing or invalid. */
        bool LoadIndexFile ( const std::filesystem::path& aIndexPath,
                             std::unordered_map<uint32_t, std::string>& aIndexTable )
        {
            std::error_code ec;
            if ( !std::filesystem::is_regular_file ( aIndexPath, ec ) )
            {
                return false;
            }
            const uintmax_t file_size = std::filesystem::file_size ( aIndexPath, ec );
            if ( ec || file_size < sizeof ( IDXHeader ) )
            {
                return false;
            }
            std::ifstream file ( aIndexPath, std::ios::in | std::ios::binary );
            if ( !file.is_open() )
            {
                return false;
            }
            IDXHeader header{};
            file.read ( reinterpret_cast<char*> ( &header ), sizeof ( header ) );
            if ( !file || std::memcmp ( header.id, "AEONIDX", 8 ) != 0 )
            {
                return false;
            }
            const uintmax_t entries_bytes = static_cast<uintmax_t> ( header.count ) * sizeof ( IDXEntry );
            if ( sizeof ( IDXHeader ) + entries_bytes > file_size )
            {
                return false;
            }
            std::vector<IDXEntry> entries ( header.count );
            if ( header.count != 0 )
            {
                file.read ( reinterpret_cast<char*> ( entries.data() ),
                            static_cast<std::streamsize> ( entries_bytes ) );
                if ( !file )
                {
                    return false;
                }
            }
            const std::streamsize string_blob_size =
                static_cast<std::streamsize> ( file_size - sizeof ( IDXHeader ) - entries_bytes );
            std::vector<char> blob ( static_cast<size_t> ( string_blob_size ) );
            if ( string_blob_size > 0 )
            {
                file.read ( blob.data(), string_blob_size );
                if ( !file )
                {
                    return false;
                }
            }
            aIndexTable.reserve ( header.count );
            for ( const IDXEntry& e : entries )
            {
                if ( e.path_offset >= blob.size() )
                {
                    return false;
                }
                // Strings are NUL-terminated; trust the producer to not run past the blob.
                aIndexTable.emplace ( e.crc, std::string ( blob.data() + e.path_offset ) );
            }
            return true;
        }

        /** Try to populate aEntries and aIndexTable by parsing an AEONPKG file
            at aPackagePath.
            @return true on success, false if the file is missing or invalid. */
        bool LoadPackageFile ( const std::filesystem::path& aPackagePath,
                               std::vector<PKGDirectoryEntry>& aEntries,
                               std::unordered_map<uint32_t, std::string>& aIndexTable )
        {
            std::error_code ec;
            if ( !std::filesystem::is_regular_file ( aPackagePath, ec ) )
            {
                return false;
            }
            const uintmax_t file_size = std::filesystem::file_size ( aPackagePath, ec );
            if ( ec || file_size < sizeof ( PKGHeader ) )
            {
                return false;
            }
            std::ifstream file ( aPackagePath, std::ios::in | std::ios::binary );
            if ( !file.is_open() )
            {
                return false;
            }
            PKGHeader header{};
            file.read ( reinterpret_cast<char*> ( &header ), sizeof ( header ) );
            if ( !file || std::memcmp ( header.id, "AEONPKG", 8 ) != 0 )
            {
                return false;
            }
            // Sanity: index/strings offsets within file, entries fit.
            const uintmax_t entries_bytes =
                static_cast<uintmax_t> ( header.file_count ) * sizeof ( PKGDirectoryEntry );
            if ( header.index_offset < sizeof ( PKGHeader ) ||
                 header.index_offset + entries_bytes > file_size ||
                 header.strings_offset < header.index_offset + entries_bytes ||
                 header.strings_offset > file_size )
            {
                return false;
            }
            // Read entries.
            aEntries.resize ( header.file_count );
            if ( header.file_count != 0 )
            {
                file.seekg ( static_cast<std::streamoff> ( header.index_offset ), std::ios::beg );
                file.read ( reinterpret_cast<char*> ( aEntries.data() ),
                            static_cast<std::streamsize> ( entries_bytes ) );
                if ( !file )
                {
                    aEntries.clear();
                    return false;
                }
            }
            // Read the string blob. Its size is bounded by either the lowest
            // data_offset (if any entries) or by EOF (empty package).
            uint64_t strings_end = file_size;
            for ( const PKGDirectoryEntry& e : aEntries )
            {
                if ( e.data_offset >= header.strings_offset && e.data_offset < strings_end )
                {
                    strings_end = e.data_offset;
                }
            }
            if ( strings_end < header.strings_offset )
            {
                return false;
            }
            const size_t blob_size = static_cast<size_t> ( strings_end - header.strings_offset );
            std::vector<char> blob ( blob_size );
            if ( blob_size != 0 )
            {
                file.seekg ( static_cast<std::streamoff> ( header.strings_offset ), std::ios::beg );
                file.read ( blob.data(), static_cast<std::streamsize> ( blob_size ) );
                if ( !file )
                {
                    aEntries.clear();
                    return false;
                }
            }
            aIndexTable.reserve ( header.file_count );
            for ( const PKGDirectoryEntry& e : aEntries )
            {
                if ( e.path_offset >= blob.size() )
                {
                    aEntries.clear();
                    aIndexTable.clear();
                    return false;
                }
                aIndexTable.emplace ( e.crc, std::string ( blob.data() + e.path_offset ) );
            }
            return true;
        }
    }

    Package::Package ( const std::string& aPath ) : mPath {aPath}, mIndexTable {}
    {
        std::error_code ec;
        if ( std::filesystem::is_regular_file ( mPath, ec ) )
        {
            if ( !LoadPackageFile ( mPath, mEntries, mIndexTable ) )
            {
                std::ostringstream oss;
                oss << "Failed to load AEONPKG package: " << mPath.string();
                throw std::runtime_error ( oss.str() );
            }
            return;
        }
        if ( std::filesystem::is_directory ( mPath ) )
        {
            // Prefer a precomputed index file (written by `aeontool index`).
            if ( LoadIndexFile ( mPath / "index.idx", mIndexTable ) )
            {
                return;
            }
            // Fall back to walking the directory tree.
            static const std::string format ( "/" );
            static const std::regex separator ( "\\\\+" );
            for ( auto& i : std::filesystem::recursive_directory_iterator ( mPath ) )
            {
                if ( std::filesystem::is_regular_file ( i ) )
                {
                    std::string location{std::regex_replace ( std::filesystem::relative ( i, mPath ).string(), separator, format ) };
                    if ( location == "index.idx" || location == "index.txt" )
                    {
                        // Skip index files produced by `aeontool index` so they
                        // don't appear as resources themselves.
                        continue;
                    }
                    mIndexTable[crc32i ( location.data(), location.size() )] = location;
                }
            }
        }
    }

    Package::~Package() = default;
    Package::Package ( Package&& aPackage ) noexcept :
        mPath ( aPackage.mPath ),
        mIndexTable ( std::move ( aPackage.mIndexTable ) ),
        mEntries ( std::move ( aPackage.mEntries ) ) {}

    const std::filesystem::path& Package::GetPath() const
    {
        return mPath;
    }
    const std::unordered_map<uint32_t, std::string>& Package::GetIndexTable() const
    {
        return mIndexTable;
    }

    namespace
    {
        const PKGDirectoryEntry* FindEntry ( const std::vector<PKGDirectoryEntry>& entries,
                                             uint32_t crc )
        {
            // Entries are sorted ascending by crc, so use binary search.
            auto it = std::lower_bound ( entries.begin(), entries.end(), crc,
                                         [] ( const PKGDirectoryEntry & e, uint32_t v )
            {
                return e.crc < v;
            } );
            if ( it == entries.end() || it->crc != crc )
            {
                return nullptr;
            }
            return &*it;
        }
    }

    size_t Package::GetFileSize ( uint32_t crc ) const
    {
        if ( !mEntries.empty() )
        {
            if ( const PKGDirectoryEntry * e = FindEntry ( mEntries, crc ) )
            {
                return static_cast<size_t> ( e->uncompressed_size );
            }
            return 0;
        }
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
        if ( !mEntries.empty() )
        {
            const PKGDirectoryEntry* e = FindEntry ( mEntries, crc );
            if ( e == nullptr )
            {
                return;
            }
            std::ifstream file ( mPath, std::ios::in | std::ios::binary );
            if ( !file.is_open() )
            {
                throw std::runtime_error ( "Could not open package file for reading." );
            }
            switch ( e->compression )
            {
            case NONE:
            {
                file.seekg ( static_cast<std::streamoff> ( e->data_offset ), std::ios::beg );
                const std::streamsize want = static_cast<std::streamsize> (
                                                 std::min<uint64_t> ( e->compressed_size, buffer_size ) );
                file.read ( reinterpret_cast<char*> ( buffer ), want );
                break;
            }
            case ZLIB:
            {
                const int ret = InflateRegion ( file, e->data_offset, e->compressed_size,
                                                buffer, buffer_size );
                if ( ret != Z_OK )
                {
                    std::ostringstream oss;
                    oss << "zlib inflate failed (code " << ret << ") for entry " << std::hex << crc;
                    throw std::runtime_error ( oss.str() );
                }
                break;
            }
            default:
            {
                std::ostringstream oss;
                oss << "Unsupported compression type " << static_cast<int> ( e->compression )
                    << " for entry " << std::hex << crc;
                throw std::runtime_error ( oss.str() );
            }
            }
            return;
        }
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

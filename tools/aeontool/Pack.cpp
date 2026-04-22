/*
Copyright (C) 2013,2018,2019,2022,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "Pack.h"
#include "aeongames/CRC.hpp"
#include "aeongames/Package.hpp"
#include "zlib.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace AeonGames
{
    namespace
    {
        constexpr int kZlibLevel = 5;
        constexpr size_t kChunk = 16384;

        /** Lowercase the string in place (ASCII only). */
        std::string ToLowerExt ( const std::filesystem::path& p )
        {
            std::string ext = p.extension().string();
            if ( !ext.empty() && ext.front() == '.' )
            {
                ext.erase ( 0, 1 );
            }
            std::transform ( ext.begin(), ext.end(), ext.begin(),
                             [] ( unsigned char c )
            {
                return static_cast<char> ( std::tolower ( c ) );
            } );
            return ext;
        }

        /** File extensions whose contents are already compressed; no point
            re-deflating them and wasting CPU at pack and load time. */
        bool ShouldStoreRaw ( const std::string& ext )
        {
            static constexpr std::array<const char*, 4> kRaw{ "png", "jpg", "jpeg", "ogg" };
            for ( const char * e : kRaw )
            {
                if ( ext == e )
                {
                    return true;
                }
            }
            return false;
        }

        /** Deflate the contents of `in` into `out`. Updates `compressed_size`
            with the number of bytes written.
            @return Z_OK on success or a zlib error code. */
        int WriteDeflated ( std::ifstream& in, std::ofstream& out, int level,
                            uint64_t& compressed_size )
        {
            compressed_size = 0;
            z_stream strm{};
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            int ret = deflateInit ( &strm, level );
            if ( ret != Z_OK )
            {
                return ret;
            }

            unsigned char inbuf[kChunk];
            unsigned char outbuf[kChunk];
            int flush = Z_NO_FLUSH;
            do
            {
                in.read ( reinterpret_cast<char*> ( inbuf ), kChunk );
                std::streamsize got = in.gcount();
                if ( in.bad() )
                {
                    deflateEnd ( &strm );
                    return Z_ERRNO;
                }
                strm.avail_in = static_cast<uInt> ( got );
                strm.next_in = inbuf;
                flush = in.eof() ? Z_FINISH : Z_NO_FLUSH;
                do
                {
                    strm.avail_out = kChunk;
                    strm.next_out = outbuf;
                    ret = deflate ( &strm, flush );
                    assert ( ret != Z_STREAM_ERROR );
                    const uInt have = kChunk - strm.avail_out;
                    if ( have != 0 )
                    {
                        out.write ( reinterpret_cast<const char*> ( outbuf ), have );
                        if ( !out )
                        {
                            deflateEnd ( &strm );
                            return Z_ERRNO;
                        }
                        compressed_size += have;
                    }
                }
                while ( strm.avail_out == 0 );
                assert ( strm.avail_in == 0 );
            }
            while ( flush != Z_FINISH );
            deflateEnd ( &strm );
            return ( ret == Z_STREAM_END ) ? Z_OK : Z_DATA_ERROR;
        }

        /** Copy the entire contents of `in` to `out`, returning the number of
            bytes copied. */
        uint64_t CopyRaw ( std::ifstream& in, std::ofstream& out )
        {
            uint64_t copied = 0;
            char buf[kChunk];
            while ( in )
            {
                in.read ( buf, kChunk );
                std::streamsize got = in.gcount();
                if ( got > 0 )
                {
                    out.write ( buf, got );
                    copied += static_cast<uint64_t> ( got );
                }
            }
            return copied;
        }

        struct PackEntry
        {
            uint32_t crc;
            std::string relative_path; /**< forward-slash relative path */
            std::filesystem::path absolute_path;
            uint64_t uncompressed_size;
            std::string ext_lower;
        };
    }

    Pack::Pack() = default;
    Pack::~Pack() = default;

    void Pack::ProcessArgs ( int argc, char** argv )
    {
        if ( argc < 2 || ( strcmp ( argv[1], "pack" ) != 0 ) )
        {
            std::ostringstream stream;
            stream << "Invalid tool name, expected pack, got "
                   << ( ( argc < 2 ) ? "nothing" : argv[1] ) << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        for ( int i = 2; i < argc; ++i )
        {
            if ( argv[i][0] == '-' )
            {
                if ( argv[i][1] == '-' )
                {
                    if ( strcmp ( &argv[i][2], "in" ) == 0 )
                    {
                        if ( ++i >= argc )
                        {
                            throw std::runtime_error ( "Missing value for --in." );
                        }
                        mInputPath = argv[i];
                    }
                    else if ( strcmp ( &argv[i][2], "out" ) == 0 )
                    {
                        if ( ++i >= argc )
                        {
                            throw std::runtime_error ( "Missing value for --out." );
                        }
                        mOutputFile = argv[i];
                    }
                    else if ( strcmp ( &argv[i][2], "extract" ) == 0 )
                    {
                        mAction = Action::Extract;
                    }
                    else if ( strcmp ( &argv[i][2], "compress" ) == 0 )
                    {
                        mAction = Action::Compress;
                    }
                    else if ( strcmp ( &argv[i][2], "directory" ) == 0 )
                    {
                        mAction = Action::Directory;
                    }
                    else if ( strcmp ( &argv[i][2], "store" ) == 0 )
                    {
                        mCompress = false;
                    }
                    else if ( strcmp ( &argv[i][2], "help" ) == 0 )
                    {
                        std::cout << "Usage: aeontool pack [options] <input>\n"
                                  << "  -c, --compress   Build an AEONPKG file (default action)\n"
                                  << "  -d, --directory  List the contents of a package or directory\n"
                                  << "  -e, --extract    Extract a package back to a directory\n"
                                  << "  -i, --in <path>  Input directory (compress) or package (extract/dir)\n"
                                  << "  -o, --out <path> Output package file (compress) or directory (extract)\n"
                                  << "      --store      Store files raw (no zlib compression)\n"
                                  << "      --help       Show this help" << std::endl;
                        return;
                    }
                }
                else
                {
                    switch ( argv[i][1] )
                    {
                    case 'i':
                        if ( ++i >= argc )
                        {
                            throw std::runtime_error ( "Missing value for -i." );
                        }
                        mInputPath = argv[i];
                        break;
                    case 'o':
                        if ( ++i >= argc )
                        {
                            throw std::runtime_error ( "Missing value for -o." );
                        }
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
                    default:
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
        if ( mAction == Action::None )
        {
            mAction = Action::Compress;
        }
    }

    int Pack::operator() ( int argc, char** argv )
    {
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
        namespace fs = std::filesystem;
        if ( !fs::is_regular_file ( mInputPath ) )
        {
            std::ostringstream oss;
            oss << "Input '" << mInputPath << "' is not a package file.";
            throw std::runtime_error ( oss.str() );
        }
        fs::path out_dir = mOutputFile.empty()
                           ? fs::path ( mInputPath ).stem()
                           : fs::path ( mOutputFile );
        if ( out_dir.empty() )
        {
            out_dir = "extracted";
        }

        Package package{mInputPath};
        const auto& index = package.GetIndexTable();
        for ( const auto& [crc, relative] : index )
        {
            const fs::path target = out_dir / relative;
            std::error_code ec;
            fs::create_directories ( target.parent_path(), ec );
            const size_t sz = package.GetFileSize ( crc );
            std::vector<char> buffer ( sz );
            package.LoadFile ( crc, buffer.data(), buffer.size() );
            std::ofstream f ( target, std::ios::out | std::ios::binary | std::ios::trunc );
            if ( !f.is_open() )
            {
                std::cout << "WARNING, could not write " << target.string() << std::endl;
                continue;
            }
            f.write ( buffer.data(), static_cast<std::streamsize> ( buffer.size() ) );
            std::cout << std::setfill ( '0' ) << std::setw ( 8 ) << std::hex
                      << crc << std::dec << " -> " << target.string()
                      << " (" << sz << " bytes)" << std::endl;
        }
        std::cout << "Extracted " << index.size() << " files to " << out_dir.string() << std::endl;
        return 0;
    }

    int Pack::ExecDirectory() const
    {
        try
        {
            Package package{mInputPath};
            for ( auto& i : package.GetIndexTable() )
            {
                std::cout << std::setfill ( '0' ) << std::setw ( 8 ) << std::hex
                          << i.first << "\t" << i.second << std::endl;
            }
        }
        catch ( const std::runtime_error& e )
        {
            std::cout << e.what();
            return -1;
        }
        return 0;
    }

    int Pack::ExecCompress()
    {
        namespace fs = std::filesystem;
        fs::path input ( mInputPath );
        if ( !fs::is_directory ( input ) )
        {
            std::ostringstream oss;
            oss << "Input '" << mInputPath << "' is not a directory.";
            throw std::runtime_error ( oss.str() );
        }
        if ( mOutputFile.empty() )
        {
            mOutputFile = input.filename().string();
            if ( mOutputFile.empty() )
            {
                mOutputFile = "package";
            }
            mOutputFile += ".pkg";
        }

        // Resolve so we can skip the output if it lives inside the input dir.
        std::error_code ec;
        const fs::path output_canonical =
            fs::weakly_canonical ( fs::absolute ( fs::path ( mOutputFile ) ), ec );

        // Pass 1: collect entries.
        std::vector<PackEntry> entries;
        for ( const auto& it : fs::recursive_directory_iterator ( input ) )
        {
            if ( !it.is_regular_file() )
            {
                continue;
            }
            // Skip the output package itself if it sits under the input dir.
            std::error_code ec2;
            const fs::path candidate = fs::weakly_canonical ( fs::absolute ( it.path() ), ec2 );
            if ( !ec && !ec2 && candidate == output_canonical )
            {
                continue;
            }
            // Skip standalone index files; they're regenerated on demand.
            const std::string filename = it.path().filename().string();
            if ( filename == "index.idx" || filename == "index.txt" )
            {
                continue;
            }

            PackEntry e;
            e.relative_path = fs::relative ( it.path(), input ).generic_string();
            e.crc = crc32i ( e.relative_path.c_str(), e.relative_path.size() );
            e.absolute_path = it.path();
            e.uncompressed_size = static_cast<uint64_t> ( fs::file_size ( it.path() ) );
            e.ext_lower = ToLowerExt ( it.path() );
            entries.push_back ( std::move ( e ) );
        }
        std::sort ( entries.begin(), entries.end(),
                    [] ( const PackEntry & a, const PackEntry & b )
        {
            return a.crc < b.crc;
        } );

        // Compute string blob and per-entry path offsets.
        std::vector<uint32_t> path_offsets ( entries.size() );
        std::vector<char> string_blob;
        for ( size_t i = 0; i < entries.size(); ++i )
        {
            path_offsets[i] = static_cast<uint32_t> ( string_blob.size() );
            string_blob.insert ( string_blob.end(),
                                 entries[i].relative_path.begin(),
                                 entries[i].relative_path.end() );
            string_blob.push_back ( '\0' );
        }

        // Header layout.
        PKGHeader header{};
        std::memcpy ( header.id, "AEONPKG", 8 );
        header.version[0] = 1;
        header.version[1] = 0;
        header.flags = 0;
        header.file_count = static_cast<uint32_t> ( entries.size() );
        header.index_offset = static_cast<uint32_t> ( sizeof ( PKGHeader ) );
        header.strings_offset = header.index_offset +
                                static_cast<uint32_t> ( entries.size() * sizeof ( PKGDirectoryEntry ) );
        header.pad = 0;
        const uint64_t data_offset_start =
            static_cast<uint64_t> ( header.strings_offset ) + string_blob.size();

        // Open output. Write header + zeroed entry table + string blob first,
        // then payloads, then seek back and rewrite the entry table with
        // populated offsets/sizes/compression.
        std::ofstream out ( mOutputFile, std::ios::out | std::ios::binary | std::ios::trunc );
        if ( !out.is_open() )
        {
            std::ostringstream oss;
            oss << "Could not open output file '" << mOutputFile << "' for writing.";
            throw std::runtime_error ( oss.str() );
        }
        out.write ( reinterpret_cast<const char*> ( &header ), sizeof ( header ) );

        std::vector<PKGDirectoryEntry> table ( entries.size() );
        for ( size_t i = 0; i < entries.size(); ++i )
        {
            table[i].crc = entries[i].crc;
            table[i].path_offset = path_offsets[i];
            table[i].uncompressed_size = entries[i].uncompressed_size;
            // data_offset / compressed_size / compression filled below.
            std::memset ( table[i].reserved, 0, sizeof ( table[i].reserved ) );
        }
        if ( !table.empty() )
        {
            out.write ( reinterpret_cast<const char*> ( table.data() ),
                        static_cast<std::streamsize> ( table.size() * sizeof ( PKGDirectoryEntry ) ) );
        }
        if ( !string_blob.empty() )
        {
            out.write ( string_blob.data(), static_cast<std::streamsize> ( string_blob.size() ) );
        }

        assert ( static_cast<uint64_t> ( out.tellp() ) == data_offset_start );

        // Pass 2: write payloads.
        for ( size_t i = 0; i < entries.size(); ++i )
        {
            const PackEntry& e = entries[i];
            std::ifstream in ( e.absolute_path, std::ios::in | std::ios::binary );
            if ( !in.is_open() )
            {
                std::cout << "WARNING, could not read " << e.absolute_path.string() << std::endl;
                table[i].data_offset = static_cast<uint64_t> ( out.tellp() );
                table[i].compressed_size = 0;
                table[i].compression = NONE;
                continue;
            }
            table[i].data_offset = static_cast<uint64_t> ( out.tellp() );
            const bool deflate_it = mCompress && !ShouldStoreRaw ( e.ext_lower );
            if ( deflate_it )
            {
                uint64_t cs = 0;
                int ret = WriteDeflated ( in, out, kZlibLevel, cs );
                if ( ret != Z_OK )
                {
                    std::cout << "Deflate failed (" << ret << ") on " << e.absolute_path.string() << std::endl;
                }
                table[i].compressed_size = cs;
                table[i].compression = ZLIB;
            }
            else
            {
                table[i].compressed_size = CopyRaw ( in, out );
                table[i].compression = NONE;
            }
            std::cout << std::setfill ( '0' ) << std::setw ( 8 ) << std::hex
                      << e.crc << std::dec << " " << e.relative_path
                      << " (" << table[i].compressed_size << "/" << e.uncompressed_size
                      << ( table[i].compression == ZLIB ? " zlib" : " raw" )
                      << ")" << std::endl;
        }

        // Rewrite the entry table with populated fields.
        out.seekp ( header.index_offset, std::ios::beg );
        if ( !table.empty() )
        {
            out.write ( reinterpret_cast<const char*> ( table.data() ),
                        static_cast<std::streamsize> ( table.size() * sizeof ( PKGDirectoryEntry ) ) );
        }
        out.close();

        std::cout << "Wrote " << entries.size() << " entries to " << mOutputFile << std::endl;
        return 0;
    }
}

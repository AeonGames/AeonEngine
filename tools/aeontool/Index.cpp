/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#include "Index.h"
#include "aeongames/CRC.hpp"
#include <algorithm>
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
#include <utility>
#include <vector>

namespace AeonGames
{
    namespace
    {
        /** On-disk header for AEONIDX index files.
            Followed by `count` sorted IndexEntry records, then a NUL-terminated
            string blob. String offsets in IndexEntry are relative to the start
            of the string blob. */
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
    }

    Index::Index() = default;
    Index::~Index() = default;

    void Index::ProcessArgs ( int argc, char** argv )
    {
        if ( argc < 2 || ( strcmp ( argv[1], "index" ) != 0 ) )
        {
            std::ostringstream stream;
            stream << "Invalid tool name, expected index, got "
                   << ( ( argc < 2 ) ? "nothing" : argv[1] ) << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        for ( int i = 2; i < argc; ++i )
        {
            if ( argv[i][0] == '-' )
            {
                if ( argv[i][1] == '-' )
                {
                    if ( strcmp ( &argv[i][2], "in" ) == 0 ||
                         strcmp ( &argv[i][2], "root" ) == 0 )
                    {
                        if ( ++i >= argc )
                        {
                            throw std::runtime_error ( "Missing value for --in/--root." );
                        }
                        mRootPath = argv[i];
                    }
                    else if ( strcmp ( &argv[i][2], "out" ) == 0 )
                    {
                        if ( ++i >= argc )
                        {
                            throw std::runtime_error ( "Missing value for --out." );
                        }
                        mOutputFile = argv[i];
                    }
                    else if ( strcmp ( &argv[i][2], "binary" ) == 0 )
                    {
                        mBinary = true;
                    }
                    else if ( strcmp ( &argv[i][2], "help" ) == 0 )
                    {
                        std::cout << "Usage: aeontool index [options] [root]\n"
                                  << "  -i, --in, --root <dir>  Root folder of cooked game files (default: game)\n"
                                  << "  -o, --out <file>        Output index file (default: <root>/index.idx in binary mode, <root>/index.txt in text mode)\n"
                                  << "  -b, --binary            Write binary AEONIDX file (sorted CRC table + string blob)\n"
                                  << "                          Default is text: '<hex crc>\\t<path>\\n' per line\n"
                                  << "      --help              Show this help" << std::endl;
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
                        mRootPath = argv[i];
                        break;
                    case 'o':
                        if ( ++i >= argc )
                        {
                            throw std::runtime_error ( "Missing value for -o." );
                        }
                        mOutputFile = argv[i];
                        break;
                    case 'b':
                        mBinary = true;
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                mRootPath = argv[i];
            }
        }
        if ( mRootPath.empty() )
        {
            throw std::runtime_error ( "No root path provided." );
        }
    }

    int Index::operator() ( int argc, char** argv )
    {
        ProcessArgs ( argc, argv );

        namespace fs = std::filesystem;
        fs::path root ( mRootPath );
        if ( !fs::exists ( root ) || !fs::is_directory ( root ) )
        {
            std::ostringstream stream;
            stream << "Root path '" << mRootPath << "' does not exist or is not a directory.";
            throw std::runtime_error ( stream.str() );
        }

        if ( mOutputFile.empty() )
        {
            // Default output lives inside the asset root so the runtime
            // Package class can find it. Binary form uses .idx, text form
            // uses .txt for human inspection.
            mOutputFile = ( root / ( mBinary ? "index.idx" : "index.txt" ) ).string();
        }

        std::ofstream out;
        out.open ( mOutputFile, mBinary ? ( std::ios::out | std::ios::binary | std::ios::trunc )
                   : ( std::ios::out | std::ios::trunc ) );
        if ( !out.is_open() )
        {
            std::ostringstream stream;
            stream << "Could not open output file '" << mOutputFile << "' for writing.";
            throw std::runtime_error ( stream.str() );
        }

        // Resolve the absolute paths of the known index files so we can skip
        // them during the directory walk (otherwise regenerating would
        // include the previous index file as an entry, and a binary run after
        // a text run would also pick up the stale index.txt and vice versa).
        std::error_code skip_ec;
        const fs::path output_canonical =
            fs::weakly_canonical ( fs::absolute ( fs::path ( mOutputFile ) ), skip_ec );
        std::error_code idx_ec;
        const fs::path idx_canonical =
            fs::weakly_canonical ( fs::absolute ( root / "index.idx" ), idx_ec );
        std::error_code txt_ec;
        const fs::path txt_canonical =
            fs::weakly_canonical ( fs::absolute ( root / "index.txt" ), txt_ec );

        auto should_skip = [&] ( const fs::path & p )
        {
            std::error_code ec;
            const fs::path candidate = fs::weakly_canonical ( fs::absolute ( p ), ec );
            if ( ec )
            {
                return false;
            }
            if ( !skip_ec && candidate == output_canonical )
            {
                return true;
            }
            if ( !idx_ec && candidate == idx_canonical )
            {
                return true;
            }
            if ( !txt_ec && candidate == txt_canonical )
            {
                return true;
            }
            return false;
        };

        if ( mBinary )
        {
            // Collect entries first so we can sort by CRC and assign string offsets.
            std::vector<std::pair<uint32_t, std::string>> entries;
            for ( const auto& entry : fs::recursive_directory_iterator ( root ) )
            {
                if ( !entry.is_regular_file() )
                {
                    continue;
                }
                if ( should_skip ( entry.path() ) )
                {
                    continue;
                }
                std::string relative = fs::relative ( entry.path(), root ).generic_string();
                uint32_t crc = crc32i ( relative.c_str(), relative.size() );
                entries.emplace_back ( crc, std::move ( relative ) );
            }
            std::sort ( entries.begin(), entries.end(),
                        [] ( const auto & a, const auto & b )
            {
                return a.first < b.first;
            } );

            IDXHeader header{};
            std::memcpy ( header.id, "AEONIDX", 8 ); // includes trailing NUL
            header.version[0] = 0;
            header.version[1] = 1;
            header.count = static_cast<uint32_t> ( entries.size() );
            out.write ( reinterpret_cast<const char*> ( &header ), sizeof ( header ) );

            // Compute string offsets relative to the start of the string blob.
            std::vector<IDXEntry> table;
            table.reserve ( entries.size() );
            uint32_t offset = 0;
            for ( const auto& e : entries )
            {
                table.push_back ( { e.first, offset } );
                offset += static_cast<uint32_t> ( e.second.size() ) + 1; // +1 for NUL
            }
            out.write ( reinterpret_cast<const char*> ( table.data() ),
                        static_cast<std::streamsize> ( table.size() * sizeof ( IDXEntry ) ) );
            for ( const auto& e : entries )
            {
                out.write ( e.second.data(), static_cast<std::streamsize> ( e.second.size() ) );
                out.put ( '\0' );
            }
            out.close();
            std::cout << "Wrote " << entries.size() << " entries to " << mOutputFile << std::endl;
            return 0;
        }

        uint32_t written = 0;
        for ( const auto& entry : fs::recursive_directory_iterator ( root ) )
        {
            if ( !entry.is_regular_file() )
            {
                continue;
            }
            if ( should_skip ( entry.path() ) )
            {
                continue;
            }
            std::string relative = fs::relative ( entry.path(), root ).generic_string();
            uint32_t crc = crc32i ( relative.c_str(), relative.size() );
            out << std::hex << std::setw ( 8 ) << std::setfill ( '0' ) << crc
                << '\t' << relative << '\n';
            ++written;
        }
        out << std::dec << std::setfill ( ' ' );

        out.close();
        std::cout << "Wrote " << written << " entries to " << mOutputFile << std::endl;
        return 0;
    }
}

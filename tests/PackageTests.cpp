/*
Copyright (C) 2018,2025,2026 Rodrigo Jose Hernandez Cordoba

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

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <filesystem>
#include <string>
#include <vector>
#include "zlib.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Package.hpp"
#include "aeongames/CRC.hpp"
#include "gtest/gtest.h"

using namespace ::testing;
namespace AeonGames
{
    class PackageTest : public ::testing::Test
    {
    protected:
        PackageTest()
        {
        }
        virtual ~PackageTest()
        {
        }
        void SetUp() override
        {
            std::filesystem::create_directory ( "package" );
            std::ofstream outfile ( "package/test.txt" );
            outfile << "This is a test file for the Package class and can be safely deleted.";
            outfile.close();
        }
        void TearDown() override
        {
            std::filesystem::remove_all ( "package" );
        }
    };
    TEST_F ( PackageTest, Directory )
    {
        Package package ( "package" );
        EXPECT_EQ ( package.GetIndexTable().size(), 1 );
        EXPECT_EQ ( package.GetIndexTable().at ( "test.txt"_crc32 ), "test.txt" );
        std::vector<char> contents ( package.GetFileSize ( "test.txt" ) + 1, 0 );
        package.LoadFile ( "test.txt", contents.data(), contents.size() - 1 );
        EXPECT_EQ ( std::string ( contents.data() ), "This is a test file for the Package class and can be safely deleted." );
    }
    TEST_F ( PackageTest, GlobalResourcesByFilename )
    {
        SetResourcePath ( {"package"} );
        std::vector<char> contents ( GetResourceSize ( "test.txt" ) + 1, 0 );
        LoadResource ( "test.txt", contents.data(), contents.size() - 1 );
        EXPECT_EQ ( std::string ( contents.data() ), "This is a test file for the Package class and can be safely deleted." );
    }
    TEST_F ( PackageTest, GlobalResourcesByCRC )
    {
        SetResourcePath ( {"package"} );
        std::vector<char> contents ( GetResourceSize ( "test.txt" ) + 1, 0 );
        LoadResource ( "test.txt"_crc32, contents.data(), contents.size() - 1 );
        EXPECT_EQ ( std::string ( contents.data() ), "This is a test file for the Package class and can be safely deleted." );
    }

    TEST ( PackageFileTest, AeonPkgRoundTrip )
    {
        // Build a tiny AEONPKG by hand: two files, one zlib-compressed, one
        // stored raw. Then open it through Package and verify both round-trip.
        const std::string text_payload =
            "Hello AEONPKG! This payload is long enough to actually benefit from "
            "deflate compression because the dictionary will pick up the repeated "
            "phrase 'AEONPKG' AEONPKG AEONPKG AEONPKG AEONPKG.";
        const std::string raw_payload = "RAW";

        const std::string text_path = "hello.txt";
        const std::string raw_path = "image.png"; // png-like ext to mark as raw

        const uint32_t crc_text = crc32i ( text_path.data(), text_path.size() );
        const uint32_t crc_raw = crc32i ( raw_path.data(), raw_path.size() );

        // String blob.
        std::vector<char> blob;
        const uint32_t off_text = static_cast<uint32_t> ( blob.size() );
        blob.insert ( blob.end(), text_path.begin(), text_path.end() );
        blob.push_back ( '\0' );
        const uint32_t off_raw = static_cast<uint32_t> ( blob.size() );
        blob.insert ( blob.end(), raw_path.begin(), raw_path.end() );
        blob.push_back ( '\0' );

        // Deflate the text payload into memory so we know its compressed size.
        std::vector<uint8_t> deflated;
        {
            z_stream strm{};
            ASSERT_EQ ( deflateInit ( &strm, 5 ), Z_OK );
            strm.next_in = reinterpret_cast<Bytef*> ( const_cast<char*> ( text_payload.data() ) );
            strm.avail_in = static_cast<uInt> ( text_payload.size() );
            uint8_t buf[1024];
            int ret;
            do
            {
                strm.next_out = buf;
                strm.avail_out = sizeof ( buf );
                ret = deflate ( &strm, Z_FINISH );
                deflated.insert ( deflated.end(), buf, buf + ( sizeof ( buf ) - strm.avail_out ) );
            }
            while ( ret == Z_OK );
            deflateEnd ( &strm );
            ASSERT_EQ ( ret, Z_STREAM_END );
        }

        PKGHeader header{};
        std::memcpy ( header.id, "AEONPKG", 8 );
        header.version[0] = 1;
        header.version[1] = 0;
        header.flags = 0;
        header.file_count = 2;
        header.index_offset = sizeof ( PKGHeader );
        header.strings_offset = header.index_offset +
                                2 * sizeof ( PKGDirectoryEntry );
        header.pad = 0;

        const uint64_t data_start = header.strings_offset + blob.size();

        // Sort entries ascending by CRC for the on-disk index.
        PKGDirectoryEntry e_text{};
        e_text.crc = crc_text;
        e_text.path_offset = off_text;
        e_text.compressed_size = deflated.size();
        e_text.uncompressed_size = text_payload.size();
        e_text.compression = ZLIB;

        PKGDirectoryEntry e_raw{};
        e_raw.crc = crc_raw;
        e_raw.path_offset = off_raw;
        e_raw.compressed_size = raw_payload.size();
        e_raw.uncompressed_size = raw_payload.size();
        e_raw.compression = NONE;

        // Decide ordering by crc, then assign data offsets in that order.
        std::vector<PKGDirectoryEntry*> ordered{ &e_text, &e_raw };
        std::sort ( ordered.begin(), ordered.end(),
                    [] ( const PKGDirectoryEntry * a, const PKGDirectoryEntry * b )
        {
            return a->crc < b->crc;
        } );
        uint64_t cursor = data_start;
        for ( PKGDirectoryEntry * e : ordered )
        {
            e->data_offset = cursor;
            cursor += e->compressed_size;
        }

        const std::string pkg_path = "roundtrip.pkg";
        {
            std::ofstream f ( pkg_path, std::ios::out | std::ios::binary | std::ios::trunc );
            ASSERT_TRUE ( f.is_open() );
            f.write ( reinterpret_cast<const char*> ( &header ), sizeof ( header ) );
            for ( const PKGDirectoryEntry * e : ordered )
            {
                f.write ( reinterpret_cast<const char*> ( e ), sizeof ( *e ) );
            }
            f.write ( blob.data(), static_cast<std::streamsize> ( blob.size() ) );
            for ( const PKGDirectoryEntry * e : ordered )
            {
                if ( e == &e_text )
                {
                    f.write ( reinterpret_cast<const char*> ( deflated.data() ),
                              static_cast<std::streamsize> ( deflated.size() ) );
                }
                else
                {
                    f.write ( raw_payload.data(),
                              static_cast<std::streamsize> ( raw_payload.size() ) );
                }
            }
        }

        Package package ( pkg_path );
        EXPECT_EQ ( package.GetIndexTable().size(), 2u );
        EXPECT_EQ ( package.GetIndexTable().at ( crc_text ), text_path );
        EXPECT_EQ ( package.GetIndexTable().at ( crc_raw ), raw_path );

        EXPECT_EQ ( package.GetFileSize ( crc_text ), text_payload.size() );
        EXPECT_EQ ( package.GetFileSize ( crc_raw ), raw_payload.size() );

        std::vector<char> got_text ( text_payload.size(), '\0' );
        package.LoadFile ( crc_text, got_text.data(), got_text.size() );
        EXPECT_EQ ( std::string ( got_text.data(), got_text.size() ), text_payload );

        std::vector<char> got_raw ( raw_payload.size(), '\0' );
        package.LoadFile ( crc_raw, got_raw.data(), got_raw.size() );
        EXPECT_EQ ( std::string ( got_raw.data(), got_raw.size() ), raw_payload );

        std::filesystem::remove ( pkg_path );
    }
}

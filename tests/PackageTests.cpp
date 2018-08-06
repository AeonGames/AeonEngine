/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#include <cstdint>
#include <functional>
#include <filesystem>
#include <string>
#include "aeongames/Package.h"
#include "aeongames/CRC.h"
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
        std::vector<char> contents ( package.GetFileSize ( "test.txt" ) );
        package.LoadFile ( "test.txt", contents.data(), contents.size() );
        EXPECT_EQ ( std::string ( contents.data() ), "This is a test file for the Package class and can be safely deleted." );
    }
}

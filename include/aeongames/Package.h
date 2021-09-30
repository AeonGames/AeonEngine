/*
Copyright (C) 2013,2018,2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PACKAGE_H
#define AEONGAMES_PACKAGE_H
/*! \file
    \brief Header for the PKG file specification.
    \author Rodrigo Hernandez.
*/
#include <cstdint>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>
#include "aeongames/Platform.h"
namespace AeonGames
{
    enum PKGCompressionTypes
    {
        NONE = 0,
        ZLIB
    };
    /// Header for PKG Files.
    struct PKGHeader
    {
        /// File ID, always "AEONPKG\0".
        char id[8];
        /// File Version version[0] = mayor, version[1] = minor.
        uint16_t version[2];
        /// Package size.
        uint32_t file_size;
        /// Number of files contained in this package.
        uint32_t file_count;
        /// String table offset.
        uint32_t string_table_offset;
    };
    struct PKGDirectoryEntry
    {
        uint32_t path;
        uint32_t offset;
        uint64_t extension_offset;
        uint64_t compressed_size;
        uint64_t uncompressed_size;
        uint64_t compression_type;
    };
    /*! \brief Package Class.
        Implements PKG file handling routines and management.
    */
    class Package
    {
    public:
        DLL Package ( const std::string& aPath );
        DLL ~Package();
        Package ( const Package& ) = delete;
        Package& operator= ( const Package& ) = delete;
        Package ( Package&& aPackage ) noexcept;
        Package& operator= ( Package&& ) = delete;
        DLL const std::filesystem::path& GetPath() const;
        /*! Returns the file size referenced by its CRC value. */
        DLL size_t GetFileSize ( uint32_t crc ) const;
        /*! Returns the file size referenced by its file name. */
        DLL size_t GetFileSize ( const std::string& aFileName ) const;
        /** Get the package index table
         * @return const reference to the package's index table.
        */
        DLL const std::unordered_map<uint32_t, std::string>& GetIndexTable() const;
        /*! Loads a specific file referenced by its CRC into the provided buffer. */
        DLL void LoadFile ( uint32_t crc, void* buffer, size_t buffer_size ) const;
        /*! Loads a specific file referenced by its path into the provided buffer. */
        DLL void LoadFile ( const std::string& aFileName, void* buffer, size_t buffer_size ) const;
    private:
        /// Package Path @note may be a package file or a directory
        const std::filesystem::path mPath;
        std::unordered_map<uint32_t, std::string> mIndexTable;
    };
}
#endif

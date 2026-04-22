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
#include "aeongames/Platform.hpp"
namespace AeonGames
{
    /** Compression types supported by PKG packages. */
    enum PKGCompressionTypes : uint8_t
    {
        NONE = 0, /**< No compression. */
        ZLIB = 1  /**< ZLIB compression. */
    };
    /** Header for AEONPKG files (version 1.x).

        On-disk layout (32 bytes, little-endian, packed naturally):
            [0..7]  char     id[8]            "AEONPKG\0"
            [8..11] uint16   version[2]       major, minor
            [12..15] uint32  flags            reserved (0)
            [16..19] uint32  file_count
            [20..23] uint32  index_offset     usually sizeof(PKGHeader)
            [24..27] uint32  strings_offset
            [28..31] uint32  pad              reserved (0)
        Followed by data_offset stored implicitly as strings_offset +
        string_blob_size, but we keep it explicit in the table below. */
    struct PKGHeader
    {
        char     id[8];          /**< File ID, always "AEONPKG\0". */
        uint16_t version[2];     /**< version[0]=major, version[1]=minor. */
        uint32_t flags;          /**< Reserved feature flags. */
        uint32_t file_count;     /**< Number of files contained in this package. */
        uint32_t index_offset;   /**< Byte offset of the entry table from start of file. */
        uint32_t strings_offset; /**< Byte offset of the string blob from start of file. */
        uint32_t pad;            /**< Reserved, must be zero. */
    };
    /** Directory entry describing a single file within an AEONPKG package.
        Entries are stored sorted ascending by `crc` to allow O(log n) lookup
        via std::lower_bound directly on the on-disk array. */
    struct PKGDirectoryEntry
    {
        uint32_t crc;               /**< CRC32 of the file path (lookup key). */
        uint32_t path_offset;       /**< Offset into the string blob (relative to strings_offset). */
        uint64_t data_offset;       /**< Absolute byte offset of the file data within the package. */
        uint64_t compressed_size;   /**< Size of the file data after compression. */
        uint64_t uncompressed_size; /**< Original size of the file data before compression. */
        uint8_t  compression;       /**< Compression algorithm (see PKGCompressionTypes). */
        uint8_t  reserved[7];       /**< Padding to 8-byte alignment, must be zero. */
    };
    static_assert ( sizeof ( PKGHeader ) == 32, "PKGHeader must be 32 bytes" );
    static_assert ( sizeof ( PKGDirectoryEntry ) == 40, "PKGDirectoryEntry must be 40 bytes" );
    /*! \brief Package Class.
        Implements PKG file handling routines and management.
    */
    class Package
    {
    public:
        /** Construct a Package from a file path.
         * @param aPath Path to the PKG file or directory to load.
         */
        DLL Package ( const std::string& aPath );
        /** Destructor. */
        DLL ~Package();
        Package ( const Package& ) = delete;
        Package& operator= ( const Package& ) = delete;
        /// @brief Move constructor.
        Package ( Package&& aPackage ) noexcept;
        Package& operator= ( Package&& ) = delete;
        /** Get the path associated with this package.
         * @return Const reference to the package's filesystem path.
         */
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
        /** PKG entries kept around when mPath is an AEONPKG file, used to
            answer GetFileSize/LoadFile without re-reading the header.
            Empty when mPath is a directory. */
        std::vector<PKGDirectoryEntry> mEntries;
    };
}
#endif

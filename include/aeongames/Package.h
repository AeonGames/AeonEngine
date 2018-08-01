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
#ifndef AEONGAMES_PACKAGE_H
#define AEONGAMES_PACKAGE_H
/*! \file
    \brief Header for the PKG file specification.
    \author Rodrigo Hernandez.
*/
#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>
//#if __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
//#endif
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
        uint32_t extension_offset;
        uint32_t compressed_size;
        uint32_t uncompressed_size;
        uint32_t compression_type;
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
        /*! Opens a Resource PKG file for reading,
        loading of individual files contained in the package file is defered to the LoadFile function. */
        DLL void Open();
        /*! Closes a previously open package file. */
        DLL void Close();
        /*! Returns the uncompressed file size referenced by its CRC value. */
        DLL uint32_t GetFileSize ( uint32_t crc ) const;
        /*! \return The amount of files contained in the package. */
        DLL uint32_t GetFileCount() const;
        /*!
        \return a constant pointer to the directory entry struct if it exists.
        \todo Rename to GetFileDirectoryEntry
        */
        DLL const PKGDirectoryEntry* ContainsFile ( uint32_t crc ) const;
        /*!
        \param crc CRC to the file path to retrieve.
        \return File path.
        */
        DLL const char* GetFilePath ( uint32_t crc ) const;
        /*!
        \param index Index of the directory entry to be retrieved
        \return A constant pointer to the directory entry struct if it exists, NULL otherwise.
        */
        DLL const PKGDirectoryEntry* GetFileByIndex ( uint32_t index ) const;
        /*! Loads a specific file referenced by its CRC into the provided buffer. */
        DLL void LoadFile ( uint32_t crc, uint8_t* buffer, uint32_t buffer_size ) const;
        //STRING_TABLE_HANDLER_DEFINITIONS()
    private:
        /// Package Path @note may be a package file or a directory
        //const std::string mPath;
        const std::experimental::filesystem::path mPath;
        /// File header
        PKGHeader mHeader;
        /// File Handle
        FILE* mHandle;
        /// Directory array
        std::vector<PKGDirectoryEntry> mDirectory{};
        std::vector<uint8_t> mStringTable{};
    };
}
#endif

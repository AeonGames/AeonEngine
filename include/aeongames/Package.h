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
#ifndef AEONGAMES_AEONPACKAGE_H
#define AEONGAMES_AEONPACKAGE_H
/*! \file
    \brief Header for the PKG file specification.
    \author Rodrigo Hernandez.
*/
#include <cstdint>

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
        /// String table offset.
        uint32_t file_size;
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
}
#endif

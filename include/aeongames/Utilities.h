/*
Copyright (C) 2017-2016 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_UTILITIES_H
#define AEONGAMES_UTILITIES_H
#include "aeongames/Platform.h"
#include <string>
#include <cstdint>

namespace AeonGames
{
    DLL const std::string GetFileExtension ( const std::string aFilePath );
    DLL bool FileExists ( const std::string& aFilePath );
    constexpr uint32_t DeBruijnSequence[32] =
    {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };
    /* find index of first set bit */
    constexpr uint32_t ffs ( uint32_t value )
    {
        return ( !value ) ? 32 : DeBruijnSequence[ ( ( ( value & ( -static_cast<int64_t> ( value ) ) ) * 0x077CB531ULL ) & 0xFFFFFFFF ) >> 27];
    }
    static_assert ( ffs ( 0x1 ) == 0, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x2 ) == 1, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x4 ) == 2, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x8 ) == 3, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x10 ) == 4, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x20 ) == 5, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x40 ) == 6, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x80 ) == 7, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x100 ) == 8, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x200 ) == 9, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x400 ) == 10, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x800 ) == 11, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x1000 ) == 12, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x2000 ) == 13, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x4000 ) == 14, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x8000 ) == 15, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x10000 ) == 16, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x20000 ) == 17, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x40000 ) == 18, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x80000 ) == 19, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x100000 ) == 20, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x200000 ) == 21, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x400000 ) == 22, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x800000 ) == 23, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x1000000 ) == 24, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x2000000 ) == 25, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x4000000 ) == 26, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x8000000 ) == 27, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x10000000 ) == 28, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x20000000 ) == 29, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x40000000 ) == 30, "Find First Bit Set Failure." );
    static_assert ( ffs ( 0x80000000 ) == 31, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x1 ) == 1, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x3 ) == 2, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x7 ) == 3, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0xf ) == 4, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x1f ) == 5, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x3f ) == 6, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x7f ) == 7, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0xff ) == 8, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x1ff ) == 9, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x3ff ) == 10, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x7ff ) == 11, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0xfff ) == 12, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x1fff ) == 13, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x3fff ) == 14, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x7fff ) == 15, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0xffff ) == 16, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x1ffff ) == 17, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x3ffff ) == 18, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x7ffff ) == 19, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0xfffff ) == 20, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x1fffff ) == 21, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x3fffff ) == 22, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x7fffff ) == 23, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0xffffff ) == 24, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x1ffffff ) == 25, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x3ffffff ) == 26, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x7ffffff ) == 27, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0xfffffff ) == 28, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x1fffffff ) == 29, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x3fffffff ) == 30, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0x7fffffff ) == 31, "Find First Bit Set Failure." );
    static_assert ( ffs ( ~0xffffffff ) == 32, "Find First Bit Set Failure." );

    /* Get count of bits set in 32bit unsigned int */
    constexpr uint32_t popcount ( uint32_t v )
    {
        return ( ( ( ( ( ( v - ( ( v >> 1 ) & 0x55555555 ) ) & 0x33333333 ) +
                       ( ( ( v - ( ( v >> 1 ) & 0x55555555 ) ) >> 2 ) & 0x33333333 ) ) +
                     ( ( ( ( v - ( ( v >> 1 ) & 0x55555555 ) ) & 0x33333333 ) +
                         ( ( ( v - ( ( v >> 1 ) & 0x55555555 ) ) >> 2 ) & 0x33333333 ) ) >> 4 ) ) &
                   0xF0F0F0F ) * 0x1010101 ) >> 24;
    }
    static_assert ( popcount ( 0x0 ) == 0, "Popcount Failure." );
    static_assert ( popcount ( 0x1 ) == 1, "Popcount Failure." );
    static_assert ( popcount ( 0x2 ) == 1, "Popcount Failure." );
    static_assert ( popcount ( 0x3 ) == 2, "Popcount Failure." );
    static_assert ( popcount ( 0x4 ) == 1, "Popcount Failure." );
    static_assert ( popcount ( 0x5 ) == 2, "Popcount Failure." );
    static_assert ( popcount ( 0x6 ) == 2, "Popcount Failure." );
    static_assert ( popcount ( 0x7 ) == 3, "Popcount Failure." );
    static_assert ( popcount ( 0x8 ) == 1, "Popcount Failure." );
    static_assert ( popcount ( 0x9 ) == 2, "Popcount Failure." );
    static_assert ( popcount ( 0xa ) == 2, "Popcount Failure." );
    static_assert ( popcount ( 0xb ) == 3, "Popcount Failure." );
    static_assert ( popcount ( 0xc ) == 2, "Popcount Failure." );
    static_assert ( popcount ( 0xd ) == 3, "Popcount Failure." );
    static_assert ( popcount ( 0xe ) == 3, "Popcount Failure." );
    static_assert ( popcount ( 0xf ) == 4, "Popcount Failure." );
}
#endif

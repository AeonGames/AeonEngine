/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include <png.h>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <cstring>
#include "PngImage.h"

namespace AeonGames
{
    struct png_read_memory_struct
    {
        const uint8_t* buffer;
        const uint8_t* pointer;
        png_size_t size;
    };

    static void png_read_memory_data ( png_structp png_ptr, png_bytep data, png_size_t length )
    {
        if ( png_ptr == nullptr )
        {
            std::cout << __FUNCTION__ << " got NULL png_ptr pointer." << std::endl;
            png_warning ( png_ptr, "Got NULL png_ptr pointer." );
            return;
        }
        auto* read_struct = static_cast<png_read_memory_struct*> ( png_get_io_ptr ( png_ptr ) );
        // Clip lenght not to get passed the end of the buffer.
        png_size_t real_length = std::min<png_size_t> ( ( ( read_struct->buffer + read_struct->size ) - read_struct->pointer ), length );
        if ( length < 1 )
        {
            std::cout << __FUNCTION__ << " tried to read past end of file" << std::endl;
            png_warning ( png_ptr, "Tried to read past end of file" );
            return;
        }
        memcpy ( data, read_struct->pointer, real_length );
        if ( real_length < length )
        {
            std::cout << __FUNCTION__ << " Returning " << real_length << " bytes instead of requested " << length << " because of end of memory" << std::endl;
            memset ( data + real_length, 0, length - real_length );
        }
        read_struct->pointer += real_length;
    }
    bool DecodePNG ( Image& aImage, size_t aBufferSize, const void* aBuffer )
    {
        if ( png_sig_cmp ( static_cast<uint8_t*> ( const_cast<void*> ( aBuffer ) ), 0, 8 ) != 0 )
        {
            return false;
        }
        try
        {
            png_structp png_ptr =
                png_create_read_struct ( PNG_LIBPNG_VER_STRING,
                                         nullptr, nullptr, nullptr );
            if ( png_ptr == nullptr )
            {
                throw std::runtime_error ( "png_create_read_struct failed." );
            }
            png_infop info_ptr = png_create_info_struct ( png_ptr );
            if ( info_ptr == nullptr )
            {
                throw std::runtime_error ( "png_create_info_struct failed." );
            }
            if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
            {
                throw std::runtime_error ( "Error during init_io." );
            }
            png_read_memory_struct read_memory_struct = {static_cast<const uint8_t*> ( aBuffer ), static_cast<const uint8_t*> ( aBuffer ) + 8,
                                                         static_cast<png_size_t> ( aBufferSize * sizeof ( uint8_t ) )
                                                        };
            png_set_read_fn ( png_ptr, &read_memory_struct, png_read_memory_data );
            png_set_sig_bytes ( png_ptr, 8 );

            png_read_info ( png_ptr, info_ptr );

            uint32_t width = png_get_image_width ( png_ptr, info_ptr );
            uint32_t height = png_get_image_height ( png_ptr, info_ptr );
            png_byte color_type = png_get_color_type ( png_ptr, info_ptr );
            png_byte bit_depth = png_get_bit_depth ( png_ptr, info_ptr );

            Image::ImageFormat format;
            Image::ImageType type;
            if ( ( color_type == PNG_COLOR_TYPE_RGB ) || ( color_type == PNG_COLOR_TYPE_RGBA ) )
            {
                format = ( color_type == PNG_COLOR_TYPE_RGB ) ? Image::ImageFormat::RGB : Image::ImageFormat::RGBA;
                type   = ( bit_depth == 8 ) ? Image::ImageType::UNSIGNED_BYTE : Image::ImageType::UNSIGNED_SHORT;
            }
            else
            {
                throw std::runtime_error ( "PNG image color type not supported...yet" );
            }

            /*int number_of_passes =*/ png_set_interlace_handling ( png_ptr );
            png_read_update_info ( png_ptr, info_ptr );

            /* read file */
            if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
            {
                throw std::runtime_error ( "Error during read_image." );
            }
            // --------------------------------------
            // This has to be changed to create a single buffer to which all row_pointers point at.
            // See http://www.piko3d.com/tutorials/libpng-tutorial-loading-png-files-from-streams
            /**@todo Add Map/Unmap functions to Image class.*/
            png_size_t rowbytes = png_get_rowbytes ( png_ptr, info_ptr );
            std::vector<uint8_t*> row_pointers ( sizeof ( png_bytep ) * height );
            aImage.Initialize ( width, height, format, type );
            uint8_t* data = static_cast<uint8_t*> ( aImage.Map() );
            for ( png_uint_32 y = 0; y < height; ++y )
            {
                row_pointers[y] = data + ( rowbytes * y );
            }
            // --------------------------------------
            png_read_image ( png_ptr, row_pointers.data() );
            png_destroy_read_struct ( &png_ptr, &info_ptr, ( png_infopp ) nullptr );
            aImage.Unmap();
        }
        catch ( std::runtime_error& e )
        {
            std::cout << e.what() << std::endl;
            return false;
        }
        return true;
    }
}

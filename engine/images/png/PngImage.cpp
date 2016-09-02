/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#include <png.h>
#include <algorithm>
#include <stdexcept>
#include "PngImage.h"

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

namespace AeonGames
{
    struct png_read_memory_struct
    {
        uint8_t* buffer;
        uint8_t* pointer;
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
        png_read_memory_struct* read_struct = static_cast<png_read_memory_struct*> ( png_get_io_ptr ( png_ptr ) );
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

    PngImage::PngImage ( const std::string & aFileName )
    {
        //--------------------------------------------------------
        // File loading code
        FILE* file = fopen ( aFileName.c_str(), "rb" );
        uint8_t* data;
        size_t dataSize;
        if ( file != nullptr )
        {
            fseek ( file, 0, SEEK_END );
            dataSize = ( size_t ) ftell ( file );
            fseek ( file, 0, SEEK_SET );
            data = static_cast<uint8_t*> ( malloc ( dataSize ) );
            if ( data == nullptr )
            {
                fclose ( file );
                throw std::runtime_error ( "Unable to allocate memory " __FUNCTION__ " " S__LINE__ );
            }
            fread ( data, dataSize, 1, file );
            fclose ( file );
        }
        else
        {
            throw std::runtime_error ( "File not found " __FUNCTION__ " " S__LINE__ );
        }
        //--------------------------------------------------------

        if ( png_sig_cmp ( data, 0, 8 ) == 0 )
        {
            png_structp png_ptr = png_create_read_struct ( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
            if ( png_ptr == NULL )
            {
                throw std::runtime_error ( "png_create_read_struct failed " __FUNCTION__ " " S__LINE__ );
            }
            png_infop info_ptr = png_create_info_struct ( png_ptr );
            if ( info_ptr == NULL )
            {
                throw std::runtime_error ( "png_create_info_struct failed " __FUNCTION__ " " S__LINE__ );
            }
            if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
            {
                throw std::runtime_error ( "Error during init_io " __FUNCTION__ " " S__LINE__ );
            }
            png_read_memory_struct read_memory_struct = {data, data + 8, dataSize};
            png_set_read_fn ( png_ptr, &read_memory_struct, png_read_memory_data );
            png_set_sig_bytes ( png_ptr, 8 );

            png_read_info ( png_ptr, info_ptr );

            mWidth = png_get_image_width ( png_ptr, info_ptr );
            mHeight = png_get_image_height ( png_ptr, info_ptr );
            png_byte color_type = png_get_color_type ( png_ptr, info_ptr );
            png_byte bit_depth = png_get_bit_depth ( png_ptr, info_ptr );

            if ( ( color_type == PNG_COLOR_TYPE_RGB ) || ( color_type == PNG_COLOR_TYPE_RGBA ) )
            {
#if 0
                ///@todo assign generic type and format.
                mFormat = ( color_type == PNG_COLOR_TYPE_RGB ) ? GL_RGB : GL_RGBA;
                mType   = ( bit_depth == 8 ) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
#endif
            }
            else
            {
                throw std::runtime_error ( "PNG image color type not supported " __FUNCTION__ " " S__LINE__ );
            }

            /*int number_of_passes =*/ png_set_interlace_handling ( png_ptr );
            png_read_update_info ( png_ptr, info_ptr );

            /* read file */
            if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
            {
                throw std::runtime_error ( "Error during read_image" __FUNCTION__ " " S__LINE__ );
            }
            // --------------------------------------
            // This has to be changed to create a single buffer to which all row_pointers point at.
            // See http://www.piko3d.com/tutorials/libpng-tutorial-loading-png-files-from-streams
            png_size_t rowbytes = png_get_rowbytes ( png_ptr, info_ptr );
            std::vector<uint8_t*> row_pointers ( sizeof ( png_bytep ) * mHeight );
            mData.resize ( rowbytes * mHeight );
            for ( png_uint_32 y = 0; y < mHeight; ++y )
            {
                row_pointers[y] = mData.data() + ( rowbytes * y );
            }
            // --------------------------------------
            png_read_image ( png_ptr, row_pointers.data() );
            png_destroy_read_struct ( &png_ptr, &info_ptr, ( png_infopp ) 0 );
        }
        else
        {
            free ( data );
            throw std::runtime_error ( "Image format not supported. " __FUNCTION__ " " S__LINE__ );
        }
        free ( data );
    }
    PngImage::~PngImage()
    {
    }
    uint32_t PngImage::Width() const
    {
        return 0;
    }
    uint32_t PngImage::Height() const
    {
        return 0;
    }
    uint32_t PngImage::Format() const
    {
        return 0;
    }
    uint32_t PngImage::Type() const
    {
        return 0;
    }
    const uint8_t* PngImage::Data() const
    {
        return 0;
    }
}
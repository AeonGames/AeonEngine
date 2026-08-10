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

#include "Window.h"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Texture.hpp"

#include <png.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace AeonGames
{
    namespace
    {
        bool WritePng ( const Texture& aTexture, const std::string& aPath )
        {
            const std::vector<uint8_t>& pixels = aTexture.GetPixels();
            const size_t stride = static_cast<size_t> ( aTexture.GetWidth() ) * 4;
            if ( pixels.size() < stride * aTexture.GetHeight() )
            {
                std::cerr << LogLevel::Error << "Screenshot buffer is shorter than the window." << std::endl;
                return false;
            }
            FILE* file = std::fopen ( aPath.c_str(), "wb" );
            if ( file == nullptr )
            {
                std::cerr << LogLevel::Error << "Unable to open " << aPath << " for writing." << std::endl;
                return false;
            }
            png_structp png = png_create_write_struct ( PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr );
            if ( png == nullptr )
            {
                std::fclose ( file );
                return false;
            }
            png_infop info = png_create_info_struct ( png );
            if ( info == nullptr )
            {
                png_destroy_write_struct ( &png, nullptr );
                std::fclose ( file );
                return false;
            }
            png_bytep* rows = nullptr;
            if ( setjmp ( png_jmpbuf ( png ) ) )
            {
                std::free ( rows );
                png_destroy_write_struct ( &png, &info );
                std::fclose ( file );
                std::cerr << LogLevel::Error << "libpng failed writing " << aPath << std::endl;
                return false;
            }
            png_init_io ( png, file );
            png_set_IHDR ( png, info, aTexture.GetWidth(), aTexture.GetHeight(), 8, PNG_COLOR_TYPE_RGBA,
                           PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
            png_write_info ( png, info );
            rows = static_cast<png_bytep*> ( std::malloc ( sizeof ( png_bytep ) * aTexture.GetHeight() ) );
            if ( rows == nullptr )
            {
                png_error ( png, "Out of memory writing screenshot." );
            }
            for ( uint32_t y = 0; y < aTexture.GetHeight(); ++y )
            {
                rows[y] = const_cast<png_bytep> ( pixels.data() + ( static_cast<size_t> ( y ) * stride ) );
            }
            png_write_image ( png, rows );
            png_write_end ( png, nullptr );
            std::free ( rows );
            png_destroy_write_struct ( &png, &info );
            std::fclose ( file );
            return true;
        }
    }

    void Window::SetScreenshot ( const std::string& aPath, uint32_t aFrame )
    {
        mScreenshotPath = aPath;
        mScreenshotFrame = aFrame;
        mFrameCounter = 0;
    }

    void Window::BeginScreenshotFrame ( void* aWindowId )
    {
        // OpenGL can only read the back buffer before EndRender swaps it, so the
        // capture has to be armed before the frame it should capture.
        if ( mScreenshotPath.empty() || mRenderer == nullptr || mFrameCounter != mScreenshotFrame )
        {
            return;
        }
        mRenderer->RequestCapture ( aWindowId );
    }

    void Window::EndScreenshotFrame ( void* aWindowId )
    {
        if ( mScreenshotPath.empty() || mRenderer == nullptr )
        {
            return;
        }
        if ( mFrameCounter++ != mScreenshotFrame )
        {
            return;
        }
        mRenderer->Finish ( aWindowId );
        Texture capture{};
        if ( !mRenderer->ReadPixels ( aWindowId, capture ) )
        {
            std::cerr << LogLevel::Error << "This renderer cannot read back the window surface." << std::endl;
            std::exit ( 1 );
        }
        const bool written = WritePng ( capture, mScreenshotPath );
        if ( written )
        {
            // Engine logging leaves std::cout in std::hex.
            std::cout << LogLevel::Info << std::dec << capture.GetWidth() << "x" << capture.GetHeight()
                      << " screenshot written to " << mScreenshotPath << std::endl;
        }
        // Matches the frametime benchmark in windows.cpp: leave from here rather
        // than threading a quit flag through three platform run loops.
        std::exit ( written ? 0 : 1 );
    }
}

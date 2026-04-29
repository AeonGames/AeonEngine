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
#include <cstring>
#include "AeonGuiOverlay.hpp"

namespace AeonGames
{
    AeonGuiOverlay::AeonGuiOverlay ( void* aWindow )
        : mWindow{}
    {
        ( void ) aWindow;
    }

    AeonGuiOverlay::~AeonGuiOverlay() = default;

    void AeonGuiOverlay::BeginFrame ( void* aWindowId, double aDelta )
    {
        ( void ) aWindowId;
        mWindow.Update ( aDelta );
    }

    void AeonGuiOverlay::EndFrame ( void* aWindowId )
    {
        ( void ) aWindowId;
        mWindow.Draw();
        // If stride matches width*4, we can use the canvas pixels directly.
        // Otherwise, copy row-by-row to remove padding.
        size_t width = mWindow.GetWidth();
        size_t height = mWindow.GetHeight();
        size_t stride = mWindow.GetStride();
        const uint8_t* src = mWindow.GetPixels();
        if ( !src || !width || !height )
        {
            mPixelBuffer.clear();
            return;
        }
        size_t row_bytes = width * 4;
        if ( stride == row_bytes )
        {
            // No padding — point directly, no copy needed.
            mPixelBuffer.clear();
        }
        else
        {
            // Strip stride padding.
            mPixelBuffer.resize ( height * row_bytes );
            for ( size_t y = 0; y < height; ++y )
            {
                memcpy ( mPixelBuffer.data() + y * row_bytes, src + y * stride, row_bytes );
            }
        }
    }

    const uint8_t* AeonGuiOverlay::GetPixels() const
    {
        // If mPixelBuffer is populated, stride padding was stripped.
        // Otherwise, the canvas pixels are usable directly.
        if ( !mPixelBuffer.empty() )
        {
            return mPixelBuffer.data();
        }
        return mWindow.GetPixels();
    }

    uint32_t AeonGuiOverlay::GetWidth() const
    {
        return static_cast<uint32_t> ( mWindow.GetWidth() );
    }

    uint32_t AeonGuiOverlay::GetHeight() const
    {
        return static_cast<uint32_t> ( mWindow.GetHeight() );
    }

    bool AeonGuiOverlay::OnMouseMove ( int32_t aX, int32_t aY )
    {
        mWindow.HandleMouseMove ( static_cast<double> ( aX ), static_cast<double> ( aY ) );
        return false;
    }

    bool AeonGuiOverlay::OnMouseButton ( int32_t aButton, bool aPressed, int32_t aX, int32_t aY )
    {
        if ( aPressed )
        {
            mWindow.HandleMouseDown ( static_cast<double> ( aX ), static_cast<double> ( aY ),
                                      static_cast<short> ( aButton ) );
        }
        else
        {
            mWindow.HandleMouseUp ( static_cast<double> ( aX ), static_cast<double> ( aY ),
                                    static_cast<short> ( aButton ) );
        }
        return false;
    }

    bool AeonGuiOverlay::OnKeyEvent ( uint32_t aKey, bool aPressed )
    {
        ( void ) aKey;
        ( void ) aPressed;
        // Key event forwarding requires mapping platform key codes to DOM key strings.
        // This will be implemented when the input mapping layer is in place.
        return false;
    }

    bool AeonGuiOverlay::OnTextInput ( uint32_t aCodepoint )
    {
        ( void ) aCodepoint;
        return false;
    }

    void AeonGuiOverlay::Resize ( uint32_t aWidth, uint32_t aHeight )
    {
        mWindow.ResizeViewport ( aWidth, aHeight );
    }

    void AeonGuiOverlay::Navigate ( const std::string& aUrl )
    {
        mWindow.location().assign ( aUrl );
    }


}

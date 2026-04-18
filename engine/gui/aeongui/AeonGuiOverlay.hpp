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
#ifndef AEONGAMES_AEONGUIOVERLAY_H
#define AEONGAMES_AEONGUIOVERLAY_H

#include <cstdint>
#include <vector>
#include "aeongames/GuiOverlay.hpp"
#include "aeongui/dom/Window.hpp"

namespace AeonGames
{
    class AeonGuiOverlay final : public GuiOverlay
    {
    public:
        AeonGuiOverlay ( void* aWindow );
        ~AeonGuiOverlay() final;

        void BeginFrame ( void* aWindowId, double aDelta ) final;
        void EndFrame ( void* aWindowId ) final;

        const uint8_t* GetPixels() const final;
        uint32_t GetWidth() const final;
        uint32_t GetHeight() const final;

        bool OnMouseMove ( int32_t aX, int32_t aY ) final;
        bool OnMouseButton ( int32_t aButton, bool aPressed, int32_t aX, int32_t aY ) final;
        bool OnKeyEvent ( uint32_t aKey, bool aPressed ) final;
        bool OnTextInput ( uint32_t aCodepoint ) final;

        void Resize ( uint32_t aWidth, uint32_t aHeight ) final;

        void Navigate ( const std::string& aUrl ) final;

    private:
        void ConvertBGRAtoRGBA();

        AeonGUI::DOM::Window mWindow;
        std::vector<uint8_t> mPixelBuffer{};
    };
}
#endif

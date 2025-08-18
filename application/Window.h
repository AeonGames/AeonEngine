/*
Copyright (C) 2017-2021,2025 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_WINDOW_H
#define AEONGAMES_WINDOW_H
#include <cstdint>
#include <memory>
#include "aeongames/Renderer.hpp"
#include "aeongames/Scene.hpp"
#ifdef __unix__
#include <X11/Xlib.h>
#endif
namespace AeonGames
{
    class Window
    {
    public:
        Window ( const std::string& aRendererName, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        ~Window();
        uint32_t Resize ( uint32_t aWidth, uint32_t aHeight );
        void Run ( Scene& aScene );
    private:
#if defined(_WIN32)
        HWND mWindowId {};
#elif defined(__unix__)
        Display* mDisplay {};
        ::Window mWindowId{None};
        Colormap mColorMap{None};
#endif
        std::unique_ptr<Renderer> mRenderer {};
        float mAspectRatio{1.0f};
    };
}
#endif

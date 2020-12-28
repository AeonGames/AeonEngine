/*
Copyright (C) 2017-2020 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_X11WINDOW_H
#define AEONGAMES_X11WINDOW_H
#ifdef __unix__
#include <X11/Xlib.h>
#include <GL/glx.h>
#include "aeongames/Platform.h"
#include "aeongames/CommonWindow.h"

namespace AeonGames
{
    class Scene;
    class X11Window : public CommonWindow
    {
    public:
        DLL X11Window ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        DLL X11Window ( void* aWindowId );
        DLL virtual ~X11Window() = 0;
        DLL void Run ( Scene& aScene ) final;
        DLL void Show ( bool aShow ) const final;
        DLL void StartRenderTimer() const final;
        DLL void StopRenderTimer() const final;
        static GLXFBConfig GetGLXConfig();
    protected:
        ::Window mWindowId{};
        Colormap mColorMap{};
    };
    using NativeWindow = X11Window;
}
#endif
#endif

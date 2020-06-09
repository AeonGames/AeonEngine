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
#if _WIN32
#ifndef AEONGAMES_WINAPIWINDOW_H
#define AEONGAMES_WINAPIWINDOW_H

#include "aeongames/CommonWindow.h"
#include <windef.h>
namespace AeonGames
{
    class Scene;
    class WinAPIWindow : public CommonWindow
    {
    public:
        DLL WinAPIWindow ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        DLL WinAPIWindow ( void* aWindowId );
        DLL virtual ~WinAPIWindow() = 0;
        DLL void Run ( Scene& aScene ) final;
        DLL void Show ( bool aShow ) const final;
        DLL void StartRenderTimer() const final;
        DLL void StopRenderTimer() const final;
    protected:
        HWND mWindowId{};
    };
    using NativeWindow = WinAPIWindow;
}
#endif
#endif
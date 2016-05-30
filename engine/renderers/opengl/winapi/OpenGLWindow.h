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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "aeongames/GameWindow.h"
#include <Windows.h>
#include <cstdint>

namespace AeonGames
{
    class AeonEngine;
    class OpenGLWindow : public GameWindow
    {
    public:
        OpenGLWindow ( AeonEngine& aAeonEngine );
        ~OpenGLWindow();
        int Run() override final;
        HWND GetWindowHandle() const;
        HDC GetDeviceContext() const;
        HGLRC GetOpenGLContext() const;
    private:
        static ATOM mWindowClassType;
        static LRESULT CALLBACK WindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
        void Initialize();
        void Finalize();
        HWND mHwnd = nullptr;
        HDC mDeviceContext = nullptr;
        HGLRC mOpenGLContext = nullptr;
        uint32_t mRefCount = 0;
        AeonEngine& mAeonEngine;
        LRESULT OnSize ( WPARAM type, WORD newwidth, WORD newheight );
        LRESULT OnPaint();
        void RenderLoop();
    };
}

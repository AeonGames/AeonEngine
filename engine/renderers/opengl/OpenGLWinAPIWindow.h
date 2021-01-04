/*
Copyright (C) 2019,2020 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLWINAPIWINDOW_H
#define AEONGAMES_OPENGLWINAPIWINDOW_H
#ifdef _WIN32
#include "OpenGLWindow.h"
namespace AeonGames
{
    class OpenGLWinAPIWindow : public OpenGLWindow
    {
    public:
        OpenGLWinAPIWindow ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        OpenGLWinAPIWindow ( const OpenGLRenderer& aOpenGLRenderer, void* aWindowId );
        ~OpenGLWinAPIWindow() final;
    private:
        void Initialize();
        void Finalize();
        bool MakeCurrent() final;
        void SwapBuffers() final;
        HDC mDeviceContext{};
    };
    using OpenGLPlatformWindow = OpenGLWinAPIWindow;
}
#endif
#endif
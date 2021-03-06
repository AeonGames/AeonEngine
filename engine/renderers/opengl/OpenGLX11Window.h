/*
Copyright (C) 2019,2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLX11WINDOW_H
#define AEONGAMES_OPENGLX11WINDOW_H
#ifdef __unix__
#include "OpenGLWindow.h"
namespace AeonGames
{
    class OpenGLX11Window final : public OpenGLWindow
    {
    public:
        OpenGLX11Window ( OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen );
        OpenGLX11Window ( OpenGLRenderer& aOpenGLRenderer, void* aWindowId );
        ~OpenGLX11Window() final;
        DLL void Run ( Scene& aScene ) final;
        DLL void Show ( bool aShow ) const final;
        DLL void StartRenderTimer() const final;
        DLL void StopRenderTimer() const final;
        static GLXFBConfig GetGLXConfig ( Display* display );
    private:
        ::Window mWindowId{};
        Colormap mColorMap{};
        void Initialize();
        void Finalize();
        bool MakeCurrent() final;
        void SwapBuffers() final;
    };
    using OpenGLPlatformWindow = OpenGLX11Window;
}
#endif
#endif

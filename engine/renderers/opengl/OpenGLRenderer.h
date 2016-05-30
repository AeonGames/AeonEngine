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
#ifndef AEONGAMES_OPENGLRENDERER_H
#define AEONGAMES_OPENGLRENDERER_H

#include "aeongames/Renderer.h"
#include <exception>
#include <vector>
#include <memory>
#ifndef _WIN32
#include <GL/gl.h>
#include <GL/glx.h>
#include "glxext.h"
#endif

#include "winapi/OpenGLWindow.h"
#include "OpenGLPipeline.h"

namespace AeonGames
{
    class OpenGLRenderer : public Renderer
    {
    public:
        OpenGLRenderer ( AeonEngine& aAeonEngine );
        ~OpenGLRenderer();
        void BeginRender() const override final;
        void EndRender() const override final;
        void Render ( const std::shared_ptr<Mesh> aMesh ) const override final;
        std::shared_ptr<Mesh> GetMesh ( const std::string& aFilename ) const override final;
        GameWindow& GetGameWindow() override final;
    private:
        void Initialize();
        void Finalize();
#ifdef _WIN32
        HINSTANCE mInstance;
        HWND mHwnd;
        HDC mDeviceContext = nullptr;
        HGLRC mOpenGLContext = nullptr;
        static LRESULT CALLBACK WindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
        static WNDPROC mWindowProc;
#else
        Display* mDisplay = nullptr;
        Window mWindow = 0;
        GLXContext mGLXContext = nullptr;
#endif
        OpenGLWindow mOpenGLWindow;
        OpenGLPipeline mOpenGLPipeline;
    };
}
#endif

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
#include "OpenGLProgram.h"

namespace AeonGames
{
    class OpenGLRenderer : public Renderer
    {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer();
        void BeginRender() const override final;
        void EndRender() const override final;
        void Render ( const std::shared_ptr<Mesh>& aMesh ) const override final;
        std::shared_ptr<Mesh> GetMesh ( const std::string& aFilename ) const override final;
        std::shared_ptr<OpenGLProgram> GetProgram ( const std::string& aFilename ) const override final;
        bool RegisterRenderingWindow ( uintptr_t aWindowId ) override final;
        void UnregisterRenderingWindow ( uintptr_t aWindowId ) override final;
        void Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const override final;
        void SetViewMatrix ( float aMatrix[16] ) override final;
        void SetProjectionMatrix ( float aMatrix[16] ) override final;
    private:
        void Initialize();
        void Finalize();
        float mViewMatrix[16] =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        float mProjectionMatrix[16] =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        float mViewProjectionMatrix[16]
        =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
#ifdef _WIN32
        HWND mHwnd;
        HDC mDeviceContext = nullptr;
        HGLRC mOpenGLContext = nullptr;
#else
        Display* mDisplay = nullptr;
        Window mWindow = 0;
        GLXContext mGLXContext = nullptr;
#endif
    };
}
extern "C"
{
    DLL AeonGames::Renderer* CreateRenderer();
    DLL void DestroyRenderer ( AeonGames::Renderer* aRenderer );
}
#endif

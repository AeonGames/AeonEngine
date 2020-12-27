/*
Copyright (C) 2016-2020 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLX11RENDERER_H
#define AEONGAMES_OPENGLX11RENDERER_H
#ifdef __unix__
#include <unordered_map>
#include <X11/Xlib.h>
#include "aeongames/Renderer.h"
#include "aeongames/Transform.h"
#include "aeongames/Matrix4x4.h"
#include "OpenGLFunctions.h"
#include "OpenGLBuffer.h"
#include "OpenGLRenderer.h"

namespace AeonGames
{
    class OpenGLX11Renderer : public OpenGLRenderer
    {
    public:
        OpenGLX11Renderer();
        ~OpenGLX11Renderer() final;
        bool MakeCurrent () const final;
        void* GetContext() const final;
        GLXFBConfig GetGLXFBConfig() const;
    private:
        void Initialize();
        void Finalize();
        /// Internal Display connection
        Display* mDisplay{XOpenDisplay ( nullptr ) };
        /// Internal GLXFBConfig
        GLXFBConfig mGLXFBConfig{};
        /// Internal OpenGL context, shared with all other contexts
        GLXContext mGLXContext{};
        /// Internal hidden OpenGL window, required because of how OpenGL works.
        ::Window mWindow{None};
        /// General VAO
        GLuint mVertexArrayObject{};
    };
    using OpenGLNativeRenderer = OpenGLX11Renderer;
}
#endif
#endif
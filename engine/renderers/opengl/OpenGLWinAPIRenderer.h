/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLWINAPIRENDERER_H
#define AEONGAMES_OPENGLWINAPIRENDERER_H
#ifdef _WIN32
#include <windef.h>
#include <unordered_map>
#include "aeongames/Renderer.h"
#include "aeongames/Transform.h"
#include "aeongames/Matrix4x4.h"
#include "OpenGLFunctions.h"
#include "OpenGLBuffer.h"
#include "OpenGLRenderer.h"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class UniformBuffer;
    class OpenGLMesh;
    class OpenGLPipeline;
    class OpenGLModel;
    class OpenGLMaterial;
    class OpenGLWinAPIRenderer final : public OpenGLRenderer
    {
    public:
        OpenGLWinAPIRenderer();
        ~OpenGLWinAPIRenderer() final;
        bool MakeCurrent() const final;
        void* GetContext() const final;
    private:
        void Initialize();
        void Finalize();
        /// Internal Window Id, required to create initial shared context
        HWND mWindow{};
        /// Internal OpenGL context, shared with all other contexts
        HGLRC mGLContext{};
        /// Internal OpenGL Device Context.
        HDC mDeviceContext{};
    };
    using OpenGLNativeRenderer = OpenGLWinAPIRenderer;
}
#endif
#endif
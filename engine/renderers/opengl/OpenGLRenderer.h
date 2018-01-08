/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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

#include <unordered_map>
#include "aeongames/Memory.h"
#include "aeongames/Renderer.h"
#include "aeongames/Transform.h"
#include "aeongames/Matrix4x4.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class OpenGLMesh;
    class OpenGLPipeline;
    class OpenGLModel;
    class OpenGLMaterial;
    class OpenGLRenderer : public Renderer, public std::enable_shared_from_this<OpenGLRenderer>
    {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer() override;
        void CacheScene ( const std::shared_ptr<Scene>& aScene ) final;
        std::unique_ptr<Window> CreateWindowProxy ( void* aWindowId ) const final;
        void* GetOpenGLContext() const;
        void* GetWindowId() const;
    private:
        void Initialize();
        void Finalize();
        /// Internal Window Id, required to create initial shared context
        void* mWindowId = nullptr;
        /// Internal OpenGL context, shared with all other contexts
        void* mOpenGLContext = nullptr;
    };
}
#endif

/*
Copyright (C) 2016-2019 Rodrigo Jose Hernandez Cordoba

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
    class UniformBuffer;
    class OpenGLMesh;
    class OpenGLPipeline;
    class OpenGLModel;
    class OpenGLMaterial;
    class OpenGLRenderer : public Renderer
    {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer() override;
        void* GetOpenGLContext() const;
        void* GetWindowId() const;
        std::unique_ptr<Window> CreateWindowProxy ( void* aWindowId ) const final;
        std::unique_ptr<Window> CreateWindowInstance ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) const final;

        std::unique_ptr<Mesh> CreateMesh ( uint32_t aPath ) const final;
        std::unique_ptr<Pipeline> CreatePipeline ( uint32_t aPath ) const final;
        std::unique_ptr<Material> CreateMaterial ( uint32_t aPath ) const final;
        std::unique_ptr<Image> CreateImage ( uint32_t aPath ) const final;
        std::unique_ptr<UniformBuffer> CreateUniformBuffer ( size_t aSize, const void* aData = nullptr ) const final;
    private:
        void Initialize();
        void Finalize();
        /// Internal Window Id, required to create initial shared context
        void* mWindowId{};
        /// Internal OpenGL context, shared with all other contexts
        void* mOpenGLContext{};
    };
}
#endif

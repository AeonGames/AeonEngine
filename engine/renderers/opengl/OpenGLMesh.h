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
#ifndef AEONGAMES_OPENGLMESH_H
#define AEONGAMES_OPENGLMESH_H

#include <exception>
#include <string>
#include "aeongames/Memory.h"
#include "aeongames/Mesh.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLMesh : public Mesh::IRenderMesh
    {
    public:
        OpenGLMesh ( const Mesh& aMesh, const std::shared_ptr<const OpenGLRenderer>& aOpenGLRenderer );
        ~OpenGLMesh() final;
        void Render() const final;
    private:
        void Initialize();
        void Finalize();
        const Mesh& mMesh;
        std::shared_ptr<const OpenGLRenderer> mOpenGLRenderer{};
        uint32_t mArray{};
        uint32_t mVertexBuffer{};
        uint32_t mIndexBuffer{};
    };
}
#endif

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

namespace AeonGames
{
    class Mesh;
    class OpenGLRenderer;
    class OpenGLMesh
    {
    public:
        OpenGLMesh ( const std::shared_ptr<const Mesh>&  aMesh, const std::shared_ptr<const OpenGLRenderer>& aOpenGLRenderer );
        ~OpenGLMesh();
        void Render() const;
    private:
        void Initialize();
        void Finalize();
        std::shared_ptr<const Mesh> mMesh;
        std::shared_ptr<const OpenGLRenderer> mOpenGLRenderer;
        uint32_t mArray = 0;
        uint32_t mVertexBuffer = 0;
        uint32_t mIndexBuffer = 0;
    };
}
#endif

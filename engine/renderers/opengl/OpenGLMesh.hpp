/*
Copyright (C) 2016-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLMESH_HPP
#define AEONGAMES_OPENGLMESH_HPP

#include <vector>
#include "OpenGLBuffer.hpp"
#include "OpenGLVariable.hpp"
namespace AeonGames
{
    class Mesh;
    class OpenGLRenderer;
    /** @brief OpenGL mesh wrapper managing vertex and index buffers. */
    class OpenGLMesh
    {
    public:
        /// @brief Construct from a renderer and mesh resource.
        OpenGLMesh ( const OpenGLRenderer&  aOpenGLRenderer, const Mesh& aMesh );
        ~OpenGLMesh();
        /// @brief Move constructor.
        OpenGLMesh ( OpenGLMesh&& aOpenGLMesh );
        OpenGLMesh ( const OpenGLMesh& aOpenGLMesh ) = delete;
        OpenGLMesh& operator= ( const OpenGLMesh& aOpenGLMesh ) = delete;
        OpenGLMesh& operator= ( OpenGLMesh&& aOpenGLMesh ) = delete;
        /// @brief Bind the vertex and index buffers.
        void Bind() const;
        /// @brief Enable the specified vertex attributes for rendering.
        void EnableAttributes ( const std::vector<OpenGLVariable>& aAttributes ) const;
        /// @brief Disable the specified vertex attributes.
        void DisableAttributes ( const std::vector<OpenGLVariable>& aAttributes ) const;
    private:
        const OpenGLRenderer& mOpenGLRenderer;
        const Mesh* mMesh{nullptr};
        OpenGLBuffer mVertexBuffer{};
        OpenGLBuffer mIndexBuffer{};
    };
}
#endif

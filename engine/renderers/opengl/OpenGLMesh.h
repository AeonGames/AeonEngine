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
#include <memory>
#include "aeongames/Mesh.h"
#include "aeongames/AABB.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLMesh : public Mesh
    {
    public:
        OpenGLMesh ( uint32_t aPath = 0 );
        ~OpenGLMesh() final;
        void Load ( uint32_t aId ) final;
        void Load ( const std::string& aFilename ) final;
        void Load ( const void* aBuffer, size_t aBufferSize ) final;
        void Load ( const MeshBuffer& aMeshBuffer ) final;
        void Unload () final;
        uint32_t GetIndexSize () const final;
        uint32_t GetIndexCount() const final;
        uint32_t GetVertexCount() const final;
        const AABB& GetAABB() const final;
        ///@name OpenGL Specific API
        ///@{
        GLenum GetIndexType() const;
        GLuint GetArrayId() const;
        GLuint GetVertexBufferId() const;
        GLuint GetIndexBufferId() const;
        ///@}
    private:
        AABB mAABB;
        uint32_t mVertexFlags{};
        uint32_t mVertexCount{};
        uint32_t mIndexCount{};
        uint32_t mIndexSize{};
        GLuint mArray{};
        GLuint mVertexBuffer{};
        GLuint mIndexBuffer{};
    };
}
#endif

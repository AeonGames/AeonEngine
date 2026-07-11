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
        /// @param aSkinnedVertexBufferId Optional GL buffer to bind as the
        ///        vertex array source in place of the mesh's rest-pose vertices
        ///        (the pre-skinned output of the compute skinning pass).
        void Bind ( GLuint aSkinnedVertexBufferId = 0 ) const;
        /// @brief Enable the specified vertex attributes for rendering.
        /// @param aBaseOffset Byte offset added to every attribute pointer, used
        ///        when the bound vertex buffer is a sub-range of a pooled buffer.
        /// @param aStrideOverride When non-zero, overrides the mesh stride; used
        ///        when drawing the compact pre-skinned buffer whose stride drops
        ///        the per-vertex weight data.
        void EnableAttributes ( const std::vector<OpenGLVariable>& aAttributes, size_t aBaseOffset = 0, size_t aStrideOverride = 0 ) const;
        /// @brief Disable the specified vertex attributes.
        void DisableAttributes ( const std::vector<OpenGLVariable>& aAttributes ) const;
        /** @brief Get the vertex buffer object id, for binding the static vertex
         * data as a storage buffer (SSBO) source in compute skinning. */
        GLuint GetVertexBufferId() const;
        /** @brief True when this (static, non-skinned) mesh's geometry lives in
         * the renderer's shared pool and is drawn with base-vertex/first-index
         * offsets rather than from its own buffers. */
        bool IsPooled() const
        {
            return mPooled;
        }
        /** @brief First vertex of this mesh within the shared per-stride vertex
         * pool (stride units); the draw call's base vertex. */
        uint32_t GetBaseVertex() const
        {
            return mBaseVertex;
        }
        /** @brief First index of this mesh within the shared uint32 index pool. */
        uint32_t GetFirstIndex() const
        {
            return mFirstIndex;
        }
    private:
        const OpenGLRenderer& mOpenGLRenderer;
        const Mesh* mMesh{nullptr};
        OpenGLBuffer mVertexBuffer{};
        OpenGLBuffer mIndexBuffer{};
        // Shared geometry pool placement (valid when mPooled). Static meshes are
        // uploaded into the renderer's shared per-stride vertex pool and shared
        // uint32 index pool at load; the draw sources them via these offsets.
        bool mPooled{false};
        uint32_t mStride{0};
        uint32_t mBaseVertex{0};
        uint32_t mFirstIndex{0};
    };
}
#endif

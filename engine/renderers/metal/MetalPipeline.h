/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_METALPIPELINE_H
#define AEONGAMES_METALPIPELINE_H

#include <memory>
#include <span>
#include "aeongames/Renderer.hpp"

namespace AeonGames
{
    class MetalBindlessResources;
    class MetalMesh;
    class Pipeline;
    struct MetalBufferBinding
    {
        uint32_t mName;
        const BufferAccessor* mBuffer;
    };
    struct MetalTextureBinding
    {
        uint32_t mName;
        void* mTexture;
        void* mSampler;
    };
    /** @brief Frame-scoped scratch memory for Metal argument buffers.
     *
     * Argument buffers are written once and read by the GPU for the rest of the
     * frame, so they are suballocated from the frame memory pools rather than
     * allocated per draw call. */
    class MetalArgumentBufferPool
    {
    public:
        struct Allocation
        {
            /** Native MTLBuffer backing the region. */
            void* mBuffer{nullptr};
            size_t mOffset{0};
        };
        virtual ~MetalArgumentBufferPool() = default;
        virtual Allocation AllocateArgumentBuffer ( size_t aSize, size_t aAlignment ) = 0;
    };
    /** @brief Compiled native Metal stages for one Pipeline resource. */
    class MetalPipeline
    {
    public:
        MetalPipeline ( void* aDevice, const Pipeline& aPipeline );
        ~MetalPipeline();
        MetalPipeline ( MetalPipeline&& ) noexcept;
        MetalPipeline& operator= ( MetalPipeline&& ) noexcept;
        MetalPipeline ( const MetalPipeline& ) = delete;
        MetalPipeline& operator= ( const MetalPipeline& ) = delete;

        void Dispatch ( void* aCommandBuffer, MetalArgumentBufferPool& aArgumentPool,
                        uint32_t aGroupCountX,
                        uint32_t aGroupCountY, uint32_t aGroupCountZ,
                        std::span<const StorageBufferBinding> aStorageBuffers,
                        uint32_t aComputeStageIndex,
                        const BufferAccessor* aClusterParams = nullptr,
                        const BufferAccessor* aMatrices = nullptr,
                        const BufferAccessor* aLights = nullptr,
                        const MetalMesh* aSourceMesh = nullptr,
                        std::span<const MetalTextureBinding> aTextures = {} ) const;
        void Draw ( void* aRenderEncoder, MetalArgumentBufferPool& aArgumentPool, const MetalMesh& aMesh,
                    std::span<const MetalBufferBinding> aBuffers,
                    std::span<const MetalTextureBinding> aTextures,
                    const MetalBindlessResources& aBindless,
                    const BufferAccessor* aSkinnedVertices,
                    Topology aTopology,
                    uint32_t aVertexStart,
                    uint32_t aVertexCount,
                    uint32_t aInstanceCount,
                    uint32_t aFirstInstance,
                    const BufferAccessor* aIndirectCommands = nullptr,
                    uint32_t aIndirectDrawCount = 0 ) const;
        void DrawFullscreen ( void* aRenderEncoder, MetalArgumentBufferPool& aArgumentPool,
                              std::span<const MetalBufferBinding> aBuffers,
                              std::span<const MetalTextureBinding> aTextures ) const;

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
#endif
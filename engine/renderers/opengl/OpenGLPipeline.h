/*
Copyright (C) 2016-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLPIPELINE_H
#define AEONGAMES_OPENGLPIPELINE_H
#include <cstdint>
#include <string>
#include "OpenGLFunctions.h"
#include "aeongames/Pipeline.h"
#include "OpenGLMaterial.h"

namespace AeonGames
{
    class BufferAccessor;
    class OpenGLBuffer;
    class OpenGLRenderer;
    class OpenGLTexture;
    class OpenGLPipeline final : public Pipeline
    {
    public:
        OpenGLPipeline ( uint32_t aPath = 0 );
        ///@name Overrides
        ///@{
        void Load ( const PipelineMsg& aPipelineMsg ) final;
        void Unload() final;
        ///@}
        ~OpenGLPipeline() final;
        void Use ( const OpenGLMaterial* aMaterial = nullptr, const BufferAccessor* aSkeletonMsg = nullptr ) const;
        GLenum GetTopology() const;
    private:
        void Initialize();
        void Finalize();
        uint32_t mProgramId{};
        GLenum mTopology{};
    };
}
#endif

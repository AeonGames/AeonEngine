/*
Copyright (C) 2016-2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include <vector>
#include <string_view>
#include "OpenGLVariable.h"
#include "OpenGLUniformBlock.h"
#include "aeongames/Pipeline.hpp"

namespace AeonGames
{
    enum  BindingLocations : uint32_t
    {
        MATRICES = 0,
        MATERIAL,
        SKELETON
    };

    class OpenGLRenderer;
    class OpenGLPipeline
    {
    public:
        OpenGLPipeline ( const OpenGLRenderer& aOpenGLRenderer, const Pipeline& aPipeline );
        OpenGLPipeline ( OpenGLPipeline&& aOpenGLPipeline );
        OpenGLPipeline ( const OpenGLPipeline& ) = delete;
        OpenGLPipeline& operator= ( const OpenGLPipeline& ) = delete;
        OpenGLPipeline& operator= ( OpenGLPipeline&& ) = delete;
        ~OpenGLPipeline();
        GLint GetProgramId() const;
        const std::vector<OpenGLVariable>& GetVertexAttributes () const;
    private:
        void ReflectAttributes();
        void ReflectUniforms();
        const OpenGLRenderer& mOpenGLRenderer;
        const Pipeline* mPipeline{};
        GLint mProgramId{};
        std::vector<OpenGLVariable> mAttributes{};
        std::vector<OpenGLVariable> mUniforms{};
        std::vector<OpenGLUniformBlock> mUniformBlocks{};
    };
}
#endif

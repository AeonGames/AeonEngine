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
#ifndef AEONGAMES_OPENGLPIPELINE_HPP
#define AEONGAMES_OPENGLPIPELINE_HPP
#include <cstdint>
#include <vector>
#include <string_view>
#include "OpenGLVariable.hpp"
#include "OpenGLUniformBlock.hpp"
#include "aeongames/Pipeline.hpp"

namespace AeonGames
{
    class OpenGLRenderer;
    /** @brief OpenGL shader program pipeline with attribute and uniform reflection. */
    class OpenGLPipeline
    {
    public:
        /// @brief Construct from a renderer and pipeline resource.
        OpenGLPipeline ( const OpenGLRenderer& aOpenGLRenderer, const Pipeline& aPipeline );
        /// @brief Move constructor.
        OpenGLPipeline ( OpenGLPipeline&& aOpenGLPipeline );
        OpenGLPipeline ( const OpenGLPipeline& ) = delete;
        OpenGLPipeline& operator= ( const OpenGLPipeline& ) = delete;
        OpenGLPipeline& operator= ( OpenGLPipeline&& ) = delete;
        ~OpenGLPipeline();
        /// @brief Get the linked shader program identifier.
        GLint GetProgramId() const;
        /// @brief Get the reflected vertex attribute descriptions.
        const std::vector<OpenGLVariable>& GetVertexAttributes () const;
        /// @brief Get a uniform block by its name hash, or nullptr if not found.
        const OpenGLUniformBlock* GetUniformBlock ( uint32_t name ) const;
        /// @brief Get the uniform location of a sampler by its name hash.
        const GLuint GetSamplerLocation ( uint32_t name_hash ) const;
    private:
        void ReflectAttributes();
        void ReflectUniforms();
        const OpenGLRenderer& mOpenGLRenderer;
        const Pipeline* mPipeline{};
        GLint mProgramId{};
        std::vector<OpenGLVariable> mAttributes{};
        std::vector<OpenGLSamplerLocation> mSamplerLocations{};
        std::vector<OpenGLUniformBlock> mUniformBlocks{};
    };
}
#endif

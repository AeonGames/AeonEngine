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
#include "aeongames/Pipeline.h"

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
        struct Attribute
        {
            uint32_t name;
            uint32_t location;
            uint32_t size;
            uint32_t type;
        };
        OpenGLPipeline ( const OpenGLRenderer& aOpenGLRenderer, const Pipeline& aPipeline );
        OpenGLPipeline ( OpenGLPipeline&& aOpenGLPipeline );
        OpenGLPipeline ( const OpenGLPipeline& ) = delete;
        OpenGLPipeline& operator= ( const OpenGLPipeline& ) = delete;
        OpenGLPipeline& operator= ( OpenGLPipeline&& ) = delete;
        ~OpenGLPipeline();
        uint32_t GetProgramId() const;
        const Attribute& GetAttribute ( uint32_t name ) const;
        const Attribute& GetAttribute ( std::string_view name ) const;
    private:
        const OpenGLRenderer& mOpenGLRenderer;
        const Pipeline* mPipeline{};
        uint32_t mProgramId{};
        std::vector<Attribute> mAttributes{};
    };
}
#endif

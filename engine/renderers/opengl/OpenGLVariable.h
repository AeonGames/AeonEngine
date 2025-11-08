/*
Copyright (C) 2025 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLVARIABLE_H
#define AEONGAMES_OPENGLVARIABLE_H

#include <cstdint>
#include "OpenGLFunctions.h"

namespace AeonGames
{
    struct OpenGLVariable
    {
        uint32_t name;
        union
        {
            GLint binding;
            GLint location;
            GLint offset;
        };
        GLint size;
        GLenum type;
    };

    struct OpenGLSamplerLocation
    {
        uint32_t name{};
        GLint location{};
    };
}
#endif

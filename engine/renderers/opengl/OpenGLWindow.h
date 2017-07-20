/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLWINDOW_H
#define AEONGAMES_OPENGLWINDOW_H

#include <cstdint>
#include <vector>
#include "aeongames/Memory.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLWindow
    {
    public:
        OpenGLWindow ( uintptr_t aWindowId, const std::shared_ptr<OpenGLRenderer> aOpenGLRenderer );
        ~OpenGLWindow();
        uintptr_t GetWindowId() const;
        void Resize ( uint32_t aWidth, uint32_t aHeight );
    private:
        void Initialize();
        void Finalize();
        uintptr_t mWindowId;
        std::shared_ptr<OpenGLRenderer> mOpenGLRenderer;
    };
}
#endif

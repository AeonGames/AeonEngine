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
#include "OpenGLWindow.h"
#include "OpenGLRenderer.h"
#include "OpenGLFunctions.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>

namespace AeonGames
{
    OpenGLWindow::OpenGLWindow ( uintptr_t aWindowId, const std::shared_ptr<OpenGLRenderer> aOpenGLRenderer ) :
        mWindowId ( aWindowId ), mOpenGLRenderer ( aOpenGLRenderer )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    OpenGLWindow::~OpenGLWindow()
    {
        Finalize();
    }

    uintptr_t OpenGLWindow::GetWindowId() const
    {
        return mWindowId;
    }

    void OpenGLWindow::Initialize()
    {
        if ( !mOpenGLRenderer )
        {
            throw std::runtime_error ( "Pointer to OpenGL Renderer is nullptr." );
        }
    }

    void OpenGLWindow::Finalize()
    {
    }

    void OpenGLWindow::Resize ( uint32_t aWidth, uint32_t aHeight )
    {
    }
}

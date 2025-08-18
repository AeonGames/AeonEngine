/*
Copyright (C) 2016-2021,2025 Rodrigo Jose Hernandez Cordoba

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
/** \File Implements the interface for the OpenGL 4.5 plugin.*/
#include <memory>
#include "aeongames/Platform.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Plugin.hpp"
#include "aeongames/StringId.hpp"
#include "OpenGLRenderer.h"

extern "C"
{
    bool OpenGLStartUp()
    {
        return AeonGames::RegisterRendererConstructor ( "OpenGL",
                [] ( void* aWindow )
        {
            return std::make_unique<AeonGames::OpenGLRenderer> ( aWindow );
        } );
    }

    void OpenGLShutdown()
    {
        AeonGames::UnregisterRendererConstructor ( "OpenGL" );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "OpenGL Renderer",
        "Implements an OpenGL 4.5 Renderer",
        OpenGLStartUp,
        OpenGLShutdown
    };
}
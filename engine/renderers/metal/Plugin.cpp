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
#include <memory>
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Platform.hpp"
#include "aeongames/Plugin.hpp"
#include "aeongames/StringId.hpp"
#include "MetalRenderer.h"

extern "C"
{
    bool MetalStartUp()
    {
        const bool legacy = AeonGames::RegisterRendererConstructor ( "Metal",
            [] ( void* aWindow )
        {
            return std::make_unique<AeonGames::MetalRenderer> ( aWindow );
        } );
        const bool configurable = AeonGames::RegisterRendererConstructorWithSettings ( "Metal",
            [] ( void* aWindow, AeonGames::RendererSettings aSettings )
        {
            return std::make_unique<AeonGames::MetalRenderer> ( aWindow, aSettings );
        } );
        return legacy && configurable;
    }

    void MetalShutdown()
    {
        AeonGames::UnregisterRendererConstructor ( "Metal" );
        AeonGames::UnregisterRendererConstructorWithSettings ( "Metal" );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "Metal Renderer",
        "Implements a native Metal 3 Renderer",
        MetalStartUp,
        MetalShutdown
    };
}
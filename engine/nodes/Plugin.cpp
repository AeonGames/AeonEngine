/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
/** \File Implements the interface for the Portaudio plugin.*/
#include "aeongames/Platform.h"
#include "aeongames/Plugin.h"
#include "aeongames/Memory.h"
#include "aeongames/Node.h"
#include "MeshNode.h"
#include <iostream>

extern "C"
{
    bool AeonEngineNodesStartUp()
    {
        AeonGames::RegisterNodeConstructor ( "Mesh", []()
        {
            return std::make_unique<AeonGames::MeshNode>();
        } );
        return true;
    }

    void AeonEngineNodesShutdown()
    {
        AeonGames::UnregisterNodeConstructor ( "Mesh" );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "AeonEngine Nodes",
        "Specialized Node Implementations",
        AeonEngineNodesStartUp,
        AeonEngineNodesShutdown
    };
}

/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
/** \File Implements the interface for the Aeon Games GUI plugin.*/
#include "aeongames/Platform.h"
#include "aeongames/GraphicalUserInterface.h"
#include "aeongames/Plugin.h"
#include "AeonGamesGUI.h"
#include <iostream>

extern "C"
{
    bool AeonGUIStartUp()
    {
        return AeonGames::RegisterGraphicalUserInterfaceConstructor ( "AeonGUI",
                [] ()
        {
            return std::make_unique<AeonGames::AeonGamesGUI>();
        } );
    }

    void AeonGUIShutdown()
    {
        AeonGames::UnregisterGraphicalUserInterfaceConstructor ( "AeonGUI" );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "AeonGUI",
        "AeonGames Graphic User Interface.",
        AeonGUIStartUp,
        AeonGUIShutdown
    };
}

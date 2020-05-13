/*
Copyright (C) 2016,2018,2020 Rodrigo Jose Hernandez Cordoba

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

#include "aeongames/AeonEngine.h"
#include "aeongames/Renderer.h"
#include "aeongames/StringId.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Window.h"
#include "aeongames/Utilities.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"
#include <cassert>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <regex>
#include <tuple>

static void GetArgumentIntoString ( const char* aArgument, void* aUserData )
{
    std::string* string_pointer = reinterpret_cast<std::string*> ( aUserData );
    if ( aArgument && string_pointer )
    {
        *string_pointer = aArgument;
    }
}

static void ArgumentExists ( const char* aArgument, void* aUserData )
{
    *reinterpret_cast<bool*> ( aUserData ) = true;
}

int Main ( int argc, char *argv[] )
{
    AeonGames::InitializeGlobalEnvironment (  argc, argv );
    {
        bool fullscreen{false};
        std::string renderer_name{};
        std::string scene_path{};
        std::array<AeonGames::OptionHandler, 3> option_handlers
        {
            AeonGames::OptionHandler{
                'r',
                "renderer",
                GetArgumentIntoString,
                &renderer_name
            },
            AeonGames::OptionHandler{
                's',
                "scene",
                GetArgumentIntoString,
                &scene_path
            },
            AeonGames::OptionHandler{
                'f',
                "fullscreen",
                ArgumentExists,
                &fullscreen
            },
        };

        ProcessOpts ( argc, argv, option_handlers.data(), option_handlers.size() );

        const AeonGames::Renderer* renderer{};
        AeonGames::Scene scene{};

        ///@todo We now have a function returning a vector of strings, use it here
        AeonGames::EnumerateRendererConstructors ( [&renderer, &renderer_name] ( const AeonGames::StringId & aIdentifier ) -> bool
        {
            if ( renderer_name.empty() || renderer_name == aIdentifier.GetString() )
            {
                renderer = AeonGames::SetRenderer ( aIdentifier.GetString() );
                return false;
            }
            return true;
        } );

        if ( renderer == nullptr )
        {
            std::cerr << AeonGames::LogLevel::Error << "No renderer available, cannot continue." << std::endl;
            AeonGames::FinalizeGlobalEnvironment();
            return -1;
        }

        /* Renderer is available from here on.*/
        if ( !scene_path.empty() )
        {
            scene.Load ( scene_path );
        }

        auto window = renderer->CreateWindowInstance ( 0, 0, 640, 480, fullscreen );
        window->Run ( scene );
    }
    AeonGames::FinalizeGlobalEnvironment();
    return 0;
}

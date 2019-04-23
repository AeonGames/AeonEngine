/*
Copyright (C) 2016,2018,2019 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>

int ENTRYPOINT main ( int argc, char *argv[] )
{
    AeonGames::InitializeGlobalEnvironment();
    {
        std::vector<std::string> renderers;
        renderers.reserve ( 5 );
        ///@todo Have a function return a vector of strings?
        AeonGames::EnumerateRendererConstructors ( [&renderers] ( const AeonGames::StringId & aIdentifier ) -> bool
        {
            renderers.emplace_back ( aIdentifier );
            std::cout << AeonGames::LogLevel::Info << aIdentifier.GetString() << std::endl;
            return true;
        } );

        if ( !renderers.size() )
        {
            std::cerr << AeonGames::LogLevel::Error << "No renderer available, cannot continue." << std::endl;
            AeonGames::FinalizeGlobalEnvironment();
            return -1;
        }
        auto renderer = AeonGames::SetRenderer ( renderers.at ( 0 ) );
        auto window = renderer->CreateWindowInstance ( 0, 0, 640, 480, false );
        window->Run();
    }
    AeonGames::FinalizeGlobalEnvironment();
    return 0;
}

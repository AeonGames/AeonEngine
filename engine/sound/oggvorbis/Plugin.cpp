/*
Copyright (C) 2017-2019,2025 Rodrigo Jose Hernandez Cordoba

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
/** \File Implements the interface for the Ogg plugin.*/
#include "aeongames/Platform.hpp"
#include "aeongames/Plugin.hpp"
#include "aeongames/Sound.hpp"
#include "OggSound.h"
#include <iostream>

extern "C"
{
    bool OggStartUp()
    {
        return AeonGames::RegisterSoundDecoder ( "OggS", AeonGames::DecodeOGG );
    }

    void OggShutdown()
    {
        AeonGames::UnregisterSoundDecoder ( "OggS" );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "Ogg sound loader",
        "Implements Ogg Vorbis sound support.",
        OggStartUp,
        OggShutdown
    };
}

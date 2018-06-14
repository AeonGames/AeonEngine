/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
/** \File Implements the interface for the PNG plugin.*/
#include "aeongames/Platform.h"
#include "aeongames/Plugin.h"
#include "PngImage.h"
#include "aeongames/Memory.h"

extern "C"
{
    bool PngStartUp()
    {
        return AeonGames::RegisterImageDecoder ( "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a", AeonGames::DecodePNG );
    }

    void PngShutdown()
    {
        AeonGames::UnregisterImageDecoder ( "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a" );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "PNG image loader",
        "Implements Portable Network Graphics image support",
        PngStartUp,
        PngShutdown
    };
}

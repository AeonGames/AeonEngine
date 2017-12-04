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
/** \File Implements the interface for the PNG plugin.*/
#include "aeongames/Platform.h"
#include "aeongames/Plugin.h"
#include "aeongames/Memory.h"
#include <iostream>
#include <portaudio.h>

extern "C"
{
    bool PortAudioStartUp()
    {
        if ( PaError error_code = Pa_Initialize() != paNoError )
        {
            std::cerr << "Error initializing PortAudio: " << Pa_GetErrorText ( error_code ) << std::endl;
            return false;
        }
        return true;
#if 0
        return AeonGames::RegisterImageLoader ( ".png",
                                                [] ( const std::string & aFilename )
        {
            return std::make_shared<AeonGames::PngImage> ( aFilename );
        } );
#endif
        return true;
    }

    void PortAudioShutdown()
    {
        if ( PaError error_code = Pa_Terminate() != paNoError )
        {
            std::cerr << "Error finalizing PortAudio: " << Pa_GetErrorText ( error_code ) << std::endl;
        }
#if 0
        AeonGames::UnregisterImageLoader ( ".png" );
#endif
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "PortAudio Sound System",
        "Implements a Sound System using the PortAudio Library",
        PortAudioStartUp,
        PortAudioShutdown
    };
}
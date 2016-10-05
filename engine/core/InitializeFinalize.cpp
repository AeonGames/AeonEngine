/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/Plugin.h"
#include "ProtoBufHelpers.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/stubs/common.h>
#include "configuration.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    static bool gInitialized = false;
    static ConfigurationBuffer gConfigurationBuffer;
#if defined(WIN32)
    static std::vector<std::tuple<HMODULE, PluginModuleInterface*>> gPlugInCache;
#else
    static std::vector<std::tuple<void*, PluginModuleInterface*>> gPlugInCache;
#endif
    static std::string gPath ( std::getenv ( "PATH" ) );

    static void LoadPlugin ( const std::string& aDir, const std::string& aFilename )
    {
        std::cout << aFilename << std::endl;
#if (defined WIN32)
        HMODULE plugin = LoadLibraryEx ( aFilename.c_str(), nullptr, 0 );
        if ( nullptr == plugin )
        {
            std::cout << "Failed to load " << aFilename << std::endl;
            return;
        }
        PluginModuleInterface* pmi = ( PluginModuleInterface* ) GetProcAddress ( ( HINSTANCE ) plugin, "PMI" );
        if ( nullptr == pmi )
        {
            std::cout << aFilename << " is not an AeonEngine Plugin." << std::endl;
            return;
        }
#else
        void* plugin = dlopen ( aFilename.c_str(), RTLD_NOW | RTLD_GLOBAL );
        if ( nullptr == game )
        {
            std::cout << "Failed to load " << aFilename << std::endl;
            return;
        }
        PluginModuleInterface* pmi = ( PluginModuleInterface* ) dlsym ( plugin, "Initialize" );
        if ( NULL == InitializeGame )
        {
            std::cout << aFilename << " is not an AeonEngine Plugin." << std::endl;
            return;
        }
#endif
        if ( pmi->StartUp() )
        {
            gPlugInCache.emplace_back ( plugin, pmi );
        }
        else
        {
#if (defined WIN32)
            FreeLibrary ( plugin );
            if ( !FreeLibrary ( ( HINSTANCE ) plugin ) )
            {
                std::cout << "FreeLibrary Failed: " << GetLastError() << std::endl;
                return;
            }
#else
            if ( dlclose ( plugin ) != 0 )
            {
                std::cout << dlerror() << std::endl;
                return;
            }
#endif
        }
    }

    bool Initialize()
    {
        if ( gInitialized )
        {
            return false;
        }
        gInitialized = true;
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        try
        {
            LoadProtoBufObject<ConfigurationBuffer> ( gConfigurationBuffer, "game/config", "AEONCFG" );
        }
        catch ( std::runtime_error e )
        {
            std::cout << "Warning: " << e.what() << std::endl;
        }
        std::cout << "PATH: " << gPath << std::endl;
        gPlugInCache.reserve ( gConfigurationBuffer.plugin_size() );
        for ( auto& i : gConfigurationBuffer.plugin() )
        {
            LoadPlugin ( gConfigurationBuffer.plugindirectory(), i );
        }
        return gInitialized;
    }

    void Finalize()
    {
        if ( !gInitialized )
        {
            return;
        }
        for ( auto& i : gPlugInCache )
        {
            std::get<1> ( i )->ShutDown();
#if (defined WIN32)
            FreeLibrary ( std::get<0> ( i ) );
#else
            dlclose ( std::get<0> ( i ) );
#endif
        }
        google::protobuf::ShutdownProtobufLibrary();
        gInitialized = false;
    }
}

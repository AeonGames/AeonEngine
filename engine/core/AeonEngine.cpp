/*
Copyright (C) 2016,2018-2022,2025 Rodrigo Jose Hernandez Cordoba

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

#include <iostream>
#include <memory>
#include <stdexcept>
#include "aeongames/ProtoBufClasses.hpp"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include <google/protobuf/stubs/common.h>
#include "configuration.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/ProtoBufHelpers.hpp"
#include "aeongames/Plugin.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Buffer.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Model.hpp"
#include "aeongames/Skeleton.hpp"
#include "aeongames/Animation.hpp"
#include "aeongames/Package.hpp"
#include "aeongames/ResourceFactory.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/Resource.hpp"
#include "Factory.h"
#ifdef __unix__
#include <X11/Xlib.h>
#include <X11/X.h>
#endif
#ifdef _WIN32
extern "C" {
    __declspec ( dllexport ) DWORD NvOptimusEnablement{1};
    __declspec ( dllexport ) int AmdPowerXpressRequestHighPerformance{1};
}
#endif

namespace AeonGames
{
    static bool gInitialized = false;
    static ConfigurationMsg gConfigurationMsg;
#if defined(WIN32)
    static std::vector<std::tuple<HMODULE, PluginModuleInterface* >> gPlugInCache;
#else
    static std::vector<std::tuple<void*, PluginModuleInterface* >> gPlugInCache;
#endif
    static std::string gPlugInPath ( std::getenv ( "PATH" ) ? std::getenv ( "PATH" ) : ""  );
    static void LoadPlugin ( const std::string& aDir, const std::string& aFilename )
    {
        std::cout << LogLevel::Info << "Plugin: " << aFilename << std::endl;
#if (defined WIN32)
        HMODULE plugin = LoadLibraryEx ( aFilename.c_str(), nullptr, 0 );
        if ( nullptr == plugin )
        {
            std::cout << LogLevel::Error << "Failed to load " << aFilename << " Error " << GetLastError() << std::endl;
            return;
        }
        auto* pmi = ( PluginModuleInterface* ) GetProcAddress ( ( HINSTANCE ) plugin, "PMI" );
        if ( nullptr == pmi )
        {
            std::cout << LogLevel::Warning << aFilename << " is not an AeonEngine Plugin." << std::endl;
            FreeLibrary ( ( HINSTANCE ) plugin );
            return;
        }
#else
        void* plugin;
        if ( ! ( plugin = dlopen ( aFilename.c_str(), RTLD_NOW | RTLD_GLOBAL ) ) )
        {
            if ( ! ( plugin = dlopen ( ( "lib" + aFilename + ".so" ).c_str(), RTLD_NOW | RTLD_GLOBAL ) ) )
            {
                std::cout << LogLevel::Error << "Failed to load " << aFilename << std::endl;
                std::cout << LogLevel::Error << "Error " << dlerror() << std::endl;
                return;
            }
        }
        PluginModuleInterface* pmi = ( PluginModuleInterface* ) dlsym ( plugin, "PMI" );
        if ( nullptr == pmi )
        {
            std::cout << aFilename << " is not an AeonEngine Plugin." << std::endl;
            dlclose ( plugin );
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

    static std::string gConfigFile{"game/config"};
    const std::array<OptionHandler, 1> gOptionHandlers
    {
        OptionHandler{
            'c',
            "config",
            [] ( const char* aArgument, void* aUserData ) -> void {
                if ( aArgument )
                {
                    std::cout << LogLevel::Info << "Reading Configuration from " << aArgument << std::endl;
                    gConfigFile = aArgument;
                }}
        }
    };

    bool InitializeGlobalEnvironment ( int argc, char *argv[] )
    {
        if ( gInitialized )
        {
            return false;
        }
        gInitialized = true;
        GOOGLE_PROTOBUF_VERIFY_VERSION;
#if _WIN32
        HANDLE hOut = GetStdHandle ( STD_OUTPUT_HANDLE );
        DWORD dwMode = 0;
        GetConsoleMode ( hOut, &dwMode );
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode ( hOut, dwMode );
#endif
        ProcessOpts ( argc, argv, gOptionHandlers.data(), gOptionHandlers.size() );
        try
        {
            LoadProtoBufObject<ConfigurationMsg> ( gConfigurationMsg, gConfigFile, "AEONCFG"_mgk );
        }
        catch ( const std::runtime_error& e )
        {
            std::cerr << LogLevel::Warning << e.what() << std::endl;
        }

        gPlugInCache.reserve ( gConfigurationMsg.plugin_size() );
        for ( auto& i : gConfigurationMsg.plugin() )
        {
            LoadPlugin ( gConfigurationMsg.plugindirectory(), i );
        }

        if ( gConfigurationMsg.package().size() )
        {
            try
            {
                SetResourcePath ( {gConfigurationMsg.package().begin(), gConfigurationMsg.package().end() } );
            }
            catch ( const std::runtime_error& e )
            {
                std::cerr << e.what() << std::endl;
            }
        }

        RegisterResourceConstructor ( "Model"_crc32,
                                      [] ( uint32_t aPath )
        {
            auto model = std::make_unique<Model>();
            model->LoadFromId ( aPath );
            return model;
        } );

        RegisterResourceConstructor ( "Skeleton"_crc32,
                                      [] ( uint32_t aPath )
        {
            auto skeleton = std::make_unique<Skeleton>();
            skeleton->LoadFromId ( aPath );
            return skeleton;
        } );
        RegisterResourceConstructor ( "Animation"_crc32,
                                      [] ( uint32_t aPath )
        {
            auto animation = std::make_unique<Animation>();
            animation->LoadFromId ( aPath );
            return animation;
        } );

        RegisterResourceConstructor ( "Texture"_crc32,
                                      [] ( uint32_t aPath )
        {
            auto texture = std::make_unique<Texture>();
            texture->LoadFromId ( aPath );
            return texture;
        } );

        RegisterResourceConstructor ( "Mesh"_crc32,
                                      [] ( uint32_t aPath )
        {
            auto mesh = std::make_unique<Mesh>();
            mesh->LoadFromId ( aPath );
            return mesh;
        } );

        RegisterResourceConstructor ( "Pipeline"_crc32,
                                      [] ( uint32_t aPath )
        {
            auto pipeline = std::make_unique<Pipeline>();
            pipeline->LoadFromId ( aPath );
            return pipeline;
        } );

        RegisterResourceConstructor ( "Material"_crc32,
                                      [] ( uint32_t aPath )
        {
            auto material = std::make_unique<Material>();
            material->LoadFromId ( aPath );
            return material;
        } );

        return gInitialized;
    }

    void FinalizeGlobalEnvironment()
    {
        if ( !gInitialized )
        {
            return;
        }
        ClearAllResources();
        // Register default resource constructors related to renderer
        UnregisterResourceConstructor ( "Texture"_crc32 );
        UnregisterResourceConstructor ( "Mesh"_crc32 );
        UnregisterResourceConstructor ( "Pipeline"_crc32 );
        UnregisterResourceConstructor ( "Material"_crc32 );
        UnregisterResourceConstructor ( "Animation"_crc32 );
        UnregisterResourceConstructor ( "Skeleton"_crc32 );
        UnregisterResourceConstructor ( "Model"_crc32 );
        for ( auto& i : gPlugInCache )
        {
            std::get<1> ( i )->ShutDown();
#if (defined WIN32)
            FreeLibrary ( std::get<0> ( i ) );
#else
            dlclose ( std::get<0> ( i ) );
#endif
        }
#if defined(__linux__) && GOOGLE_PROTOBUF_VERSION > 3006001
        // protobuf 3.6.1 on Linux has a bug in the Shutdown code
        google::protobuf::ShutdownProtobufLibrary();
#endif
        gInitialized = false;
    }

    static std::vector<Package> gResourcePath{};
    std::vector<std::string> GetResourcePath()
    {
        std::vector<std::string> path;
        path.reserve ( gResourcePath.size() );
        for ( auto& i : gResourcePath )
        {
            path.emplace_back ( i.GetPath().string() );
        }
        return path;
    }
    void SetResourcePath ( const std::vector<std::string>& aPath )
    {
        gResourcePath.clear();
        gResourcePath.reserve ( aPath.size() );
        std::ostringstream stream;
        for ( auto& i : aPath )
        {
            try
            {
                gResourcePath.emplace_back ( i );
            }
            catch ( const std::runtime_error& e )
            {
                stream << e.what() << std::endl;
            }
            catch ( ... )
            {
                throw;
            }
        }
        if ( stream.rdbuf()->in_avail() > 0 )
        {
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    size_t GetResourceSize ( uint32_t crc )
    {
        for ( auto &i : gResourcePath )
        {
            auto resource = i.GetIndexTable().find ( crc );
            if ( resource != i.GetIndexTable().end() )
            {
                return i.GetFileSize ( crc );
            }
        }
        return 0;
    }

    std::string GetResourcePath ( uint32_t crc )
    {
        for ( auto &i : gResourcePath )
        {
            auto resource = i.GetIndexTable().find ( crc );
            if ( resource != i.GetIndexTable().end() )
            {
                return resource->second;
            }
        }
        return std::string{};
    }

    size_t GetResourceSize ( const std::string& aFileName )
    {
        return GetResourceSize ( crc32i ( aFileName.data(), aFileName.size() ) );
    }
    void LoadResource ( uint32_t crc, void* buffer, size_t buffer_size )
    {
        for ( auto &i : gResourcePath )
        {
            auto resource = i.GetIndexTable().find ( crc );
            if ( resource != i.GetIndexTable().end() )
            {
                i.LoadFile ( crc, buffer, buffer_size );
                return;
            }
        }
        throw std::runtime_error ( "Resource not found." );
    }
    void LoadResource ( const std::string& aFileName, void* buffer, size_t buffer_size )
    {
        LoadResource ( crc32i ( aFileName.data(), aFileName.size() ), buffer, buffer_size );
    }
}

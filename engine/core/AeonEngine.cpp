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
#include <iostream>
#include <utility>
#include <memory>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/Plugin.h"
#include "aeongames/Node.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/stubs/common.h>
#include "configuration.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/AeonEngine.h"
#include "aeongames/Renderer.h"
#include "aeongames/UniformBuffer.h"
#include "aeongames/Image.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Model.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Package.h"
#include "aeongames/ResourceFactory.h"
#include "Factory.h"

namespace AeonGames
{
    static bool gInitialized = false;
    static std::unique_ptr<Renderer> gRenderer{};
    static ConfigurationBuffer gConfigurationBuffer;
#if defined(WIN32)
    static std::vector<std::tuple<HMODULE, PluginModuleInterface*>> gPlugInCache;
#else
    static std::vector<std::tuple<void*, PluginModuleInterface*>> gPlugInCache;
#endif
    static std::string gPlugInPath ( std::getenv ( "PATH" ) ? std::getenv ( "PATH" ) : ""  );
    static void LoadPlugin ( const std::string& aDir, const std::string& aFilename )
    {
        std::cout << "Plugin: " << aFilename << std::endl;
#if (defined WIN32)
        HMODULE plugin = LoadLibraryEx ( aFilename.c_str(), nullptr, 0 );
        if ( nullptr == plugin )
        {
            std::cout << "Failed to load " << aFilename << " Error " << GetLastError() << std::endl;
            return;
        }
        auto* pmi = ( PluginModuleInterface* ) GetProcAddress ( ( HINSTANCE ) plugin, "PMI" );
        if ( nullptr == pmi )
        {
            std::cout << aFilename << " is not an AeonEngine Plugin." << std::endl;
            FreeLibrary ( ( HINSTANCE ) plugin );
            return;
        }
#else
        void* plugin;
        if ( ! ( plugin = dlopen ( aFilename.c_str(), RTLD_NOW | RTLD_GLOBAL ) ) )
        {
            if ( ! ( plugin = dlopen ( ( "lib" + aFilename + ".so" ).c_str(), RTLD_NOW | RTLD_GLOBAL ) ) )
            {
                std::cout << "Failed to load " << aFilename << std::endl;
                std::cout << "Error " << dlerror() << std::endl;
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

    bool InitializeGlobalEnvironment()
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
        catch ( std::runtime_error& e )
        {
            std::cerr << "Warning: " << e.what() << std::endl;
        }

        gPlugInCache.reserve ( gConfigurationBuffer.plugin_size() );
        for ( auto& i : gConfigurationBuffer.plugin() )
        {
            LoadPlugin ( gConfigurationBuffer.plugindirectory(), i );
        }

        if ( gConfigurationBuffer.package().size() )
        {
            try
            {
                SetResourcePath ( {gConfigurationBuffer.package().begin(), gConfigurationBuffer.package().end() } );
            }
            catch ( std::runtime_error& e )
            {
                std::cerr << e.what() << std::endl;
            }
        }

        RegisterResourceConstructor ( "Model"_crc32,
                                      [] ( uint32_t aPath ) -> UniqueAnyPtr
        {
            return MakeUniqueAny<Model> ( aPath );
        } );

        RegisterResourceConstructor ( "Skeleton"_crc32,
                                      [] ( uint32_t aPath ) -> UniqueAnyPtr
        {
            return MakeUniqueAny<Skeleton> ( aPath );
        } );
        RegisterResourceConstructor ( "Animation"_crc32,
                                      [] ( uint32_t aPath ) -> UniqueAnyPtr
        {
            return MakeUniqueAny<Animation> ( aPath );
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
        if ( gRenderer )
        {
            // Register default resource constructors related to renderer
            UnregisterResourceConstructor ( "Image"_crc32 );
            UnregisterResourceConstructor ( "Mesh"_crc32 );
            UnregisterResourceConstructor ( "Pipeline"_crc32 );
            UnregisterResourceConstructor ( "Material"_crc32 );
        }
        UnregisterResourceConstructor ( "Animation"_crc32 );
        UnregisterResourceConstructor ( "Skeleton"_crc32 );
        UnregisterResourceConstructor ( "Model"_crc32 );
        /* The renderer code must reside in plugin address space,
         so reset before unloading any plugins. */
        gRenderer.reset();
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
            catch ( std::runtime_error& e )
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

    // Renderer------------------------------------------------------------------------------
    UniformBuffer::~UniformBuffer() //= default;
    {
        std::cout << __func__ << std::endl;
    }
    const Renderer* GetRenderer()
    {
        return gRenderer.get();
    }
    const Renderer* SetRenderer ( const std::string& aIdentifier )
    {
        if ( gRenderer )
        {
            throw std::runtime_error ( "Global renderer already set." );
        }
        gRenderer = ConstructRenderer ( aIdentifier );

        // Register default resource constructors related to renderer
        RegisterResourceConstructor ( "Image"_crc32,
                                      [] ( uint32_t aPath )
        {
            return GetRenderer()->CreateImage ( aPath );
        } );

        RegisterResourceConstructor ( "Mesh"_crc32,
                                      [] ( uint32_t aPath )
        {
            return GetRenderer()->CreateMesh ( aPath );
        } );

        RegisterResourceConstructor ( "Pipeline"_crc32,
                                      [] ( uint32_t aPath )
        {
            return GetRenderer()->CreatePipeline ( aPath );
        } );

        RegisterResourceConstructor ( "Material"_crc32,
                                      [] ( uint32_t aPath )
        {
            return GetRenderer()->CreateMaterial ( aPath );
        } );

        return gRenderer.get();
    }
}

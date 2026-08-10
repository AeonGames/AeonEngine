/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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

#include "aeongames/UserPath.hpp"

#include <cstdlib>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <vector>
#else
#include <unistd.h>
#include <vector>
#endif

namespace AeonGames
{
    namespace
    {
        constexpr const char* kOrganization = "AeonGames";
        std::string gApplicationName{"AeonEngine"};
        std::filesystem::path gOverride{};

        std::filesystem::path EnvironmentPath ( const char* aName )
        {
            const char* value = std::getenv ( aName );
            return ( value != nullptr && value[0] != '\0' ) ? std::filesystem::path{value} :
                   std::filesystem::path{};
        }

        std::filesystem::path ExecutableDirectory()
        {
#if defined(_WIN32)
            std::wstring buffer ( MAX_PATH, L'\0' );
            const DWORD length = GetModuleFileNameW ( nullptr, buffer.data(), static_cast<DWORD> ( buffer.size() ) );
            if ( length == 0 )
            {
                return {};
            }
            buffer.resize ( length );
            return std::filesystem::path{buffer}.parent_path();
#elif defined(__APPLE__)
            uint32_t size = 0;
            _NSGetExecutablePath ( nullptr, &size );
            std::vector<char> buffer ( size + 1, '\0' );
            if ( _NSGetExecutablePath ( buffer.data(), &size ) != 0 )
            {
                return {};
            }
            std::error_code error{};
            const std::filesystem::path resolved = std::filesystem::canonical ( buffer.data(), error );
            return error ? std::filesystem::path{buffer.data() } .parent_path() : resolved.parent_path();
#else
            std::error_code error {};
            const std::filesystem::path resolved = std::filesystem::read_symlink ( "/proc/self/exe", error );
            return error ? std::filesystem::path{} :
                   resolved.parent_path();
#endif
        }

        /** A marker file next to the executable keeps everything in the install
            tree, which is what a portable or USB install wants and what makes a
            development checkout self contained. */
        std::filesystem::path PortableRoot()
        {
            const std::filesystem::path directory = ExecutableDirectory();
            if ( directory.empty() )
            {
                return {};
            }
            std::error_code error{};
            if ( !std::filesystem::exists ( directory / "portable", error ) || error )
            {
                return {};
            }
            return directory / "user";
        }

        std::filesystem::path HomePath()
        {
            const std::filesystem::path home = EnvironmentPath ( "HOME" );
            if ( !home.empty() )
            {
                return home;
            }
#if defined(_WIN32)
            return EnvironmentPath ( "USERPROFILE" );
#else
            return std::filesystem::temp_directory_path();
#endif
        }

        std::filesystem::path PlatformRoot ( UserDirectory aDirectory )
        {
#if defined(_WIN32)
            // Local rather than roaming: saves and world state are large and
            // machine specific, which is exactly what roaming profiles are not for.
            PWSTR folder = nullptr;
            std::filesystem::path base{};
            if ( SUCCEEDED ( SHGetKnownFolderPath ( FOLDERID_LocalAppData, 0, nullptr, &folder ) ) )
            {
                base = std::filesystem::path{folder};
            }
            CoTaskMemFree ( folder );
            if ( base.empty() )
            {
                base = EnvironmentPath ( "LOCALAPPDATA" );
            }
            return base / ( aDirectory == UserDirectory::Cache ? "cache" : "" );
#elif defined(__APPLE__)
            const std::filesystem::path home = HomePath();
            switch ( aDirectory )
            {
            case UserDirectory::Cache:
                return home / "Library" / "Caches";
            case UserDirectory::Config:
            case UserDirectory::Data:
            default:
                return home / "Library" / "Application Support";
            }
#else
            // XDG base directories. A dotted directory straight in $HOME is the
            // older convention and is what the specification exists to replace.
            const std::filesystem::path home = HomePath();
            switch ( aDirectory )
            {
            case UserDirectory::Config:
            {
                const std::filesystem::path configured = EnvironmentPath ( "XDG_CONFIG_HOME" );
                return configured.empty() ? home / ".config" : configured;
            }
            case UserDirectory::Cache:
            {
                const std::filesystem::path configured = EnvironmentPath ( "XDG_CACHE_HOME" );
                return configured.empty() ? home / ".cache" : configured;
            }
            case UserDirectory::Data:
            default:
            {
                const std::filesystem::path configured = EnvironmentPath ( "XDG_DATA_HOME" );
                return configured.empty() ? home / ".local" / "share" : configured;
            }
            }
#endif
        }

        const char* SubDirectory ( UserDirectory aDirectory )
        {
            switch ( aDirectory )
            {
            case UserDirectory::Config:
                return "config";
            case UserDirectory::Cache:
                return "cache";
            case UserDirectory::Data:
            default:
                return "data";
            }
        }
    }

    void SetApplicationName ( const std::string& aName )
    {
        gApplicationName = aName;
    }

    const std::string& GetApplicationName()
    {
        return gApplicationName;
    }

    void SetUserPathOverride ( const std::filesystem::path& aPath )
    {
        gOverride = aPath;
    }

    std::filesystem::path GetUserPath ( UserDirectory aDirectory )
    {
        std::filesystem::path root = gOverride;
        if ( root.empty() )
        {
            root = EnvironmentPath ( "AEON_USER_PATH" );
        }
        if ( root.empty() )
        {
            root = PortableRoot();
        }
        // Only the platform layout separates the three by location; an override
        // is one directory, so the kinds are kept apart by name underneath it.
        const std::filesystem::path path = root.empty() ?
                                           PlatformRoot ( aDirectory ) / kOrganization / gApplicationName :
                                           root / SubDirectory ( aDirectory );
        std::error_code error{};
        std::filesystem::create_directories ( path, error );
        if ( error && !std::filesystem::exists ( path ) )
        {
            throw std::runtime_error ( "Unable to create the user directory " + path.string() );
        }
        return path;
    }
}

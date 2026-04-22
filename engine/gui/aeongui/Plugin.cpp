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
#include <memory>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <vector>
#include "aeongames/Platform.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Plugin.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/GuiOverlay.hpp"
#include "aeongui/AeonGUI.hpp"
#include "aeongui/ResourceLoader.hpp"
#include "AeonGuiOverlay.hpp"

namespace
{
    // Normalize a path or URL into a candidate package-relative key:
    //  - strip any "file://" scheme
    //  - convert backslashes to forward slashes
    //  - drop a leading slash before a Windows drive letter ("/C:/foo" -> "C:/foo")
    //  - lexically normalize
    std::string NormalizeForLookup ( const std::string& aPathOrUrl )
    {
        std::string s = aPathOrUrl;
        if ( s.rfind ( "file://", 0 ) == 0 )
        {
            s.erase ( 0, 7 );
        }
        std::replace ( s.begin(), s.end(), '\\', '/' );
        if ( s.size() > 2 && s[0] == '/' &&
             std::isalpha ( static_cast<unsigned char> ( s[1] ) ) && s[2] == ':' )
        {
            s.erase ( 0, 1 );
        }
        return s;
    }

    // Try to express @p aAbsolute as a path relative to @p aRoot using
    // forward slashes and no leading "./" components. Returns empty string
    // when @p aAbsolute is not under @p aRoot.
    std::string MakeRelativeToRoot ( const std::filesystem::path& aRoot,
                                     const std::filesystem::path& aAbsolute )
    {
        std::error_code ec;
        std::filesystem::path rel = std::filesystem::relative ( aAbsolute, aRoot, ec );
        if ( ec || rel.empty() )
        {
            return {};
        }
        const std::string s = rel.generic_string();
        if ( s.empty() || s.rfind ( "..", 0 ) == 0 )
        {
            // Not actually under aRoot.
            return {};
        }
        return s;
    }

    bool PackageResourceLoader ( const std::string& aPathOrUrl,
                                 std::vector<uint8_t>& aBytes )
    {
        const std::string normalized = NormalizeForLookup ( aPathOrUrl );

        // First try the path verbatim (package keys are forward-slash relative
        // strings; this covers cases where AeonGUI was navigated using a key
        // we passed in directly).
        size_t size = AeonGames::GetResourceSize ( normalized );
        std::string key = normalized;

        // Otherwise, see if the absolute path lies under any registered
        // resource path (package directory) and use the relative form.
        if ( size == 0 )
        {
            std::filesystem::path absolute{normalized};
            std::error_code ec;
            if ( absolute.is_absolute() )
            {
                absolute = absolute.lexically_normal();
                for ( const std::string& root : AeonGames::GetResourcePath() )
                {
                    std::filesystem::path rootPath{root};
                    rootPath = rootPath.lexically_normal();
                    std::string rel = MakeRelativeToRoot ( rootPath, absolute );
                    if ( rel.empty() )
                    {
                        continue;
                    }
                    size = AeonGames::GetResourceSize ( rel );
                    if ( size != 0 )
                    {
                        key = std::move ( rel );
                        break;
                    }
                }
            }
        }

        if ( size == 0 )
        {
            return false;
        }

        aBytes.resize ( size );
        try
        {
            AeonGames::LoadResource ( key, aBytes.data(), aBytes.size() );
        }
        catch ( ... )
        {
            return false;
        }
        return true;
    }
}

extern "C"
{
    bool AeonGUIStartUp()
    {
        AeonGUI::Initialize ( 0, nullptr );
        AeonGUI::SetResourceLoader ( PackageResourceLoader );
        return AeonGames::RegisterGuiOverlayConstructor ( "AeonGUI",
                [] ( void* aWindow )
        {
            return std::make_unique<AeonGames::AeonGuiOverlay> ( aWindow );
        } );
    }

    void AeonGUIShutDown()
    {
        AeonGames::UnregisterGuiOverlayConstructor ( "AeonGUI" );
        AeonGUI::SetResourceLoader ( {} );
        AeonGUI::Finalize();
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "AeonGUI Overlay",
        "Provides GUI overlay rendering using the AeonGUI library",
        AeonGUIStartUp,
        AeonGUIShutDown
    };
}

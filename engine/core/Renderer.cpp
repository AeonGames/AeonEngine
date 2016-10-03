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
#include "aeongames/Renderer.h"
#include "aeongames/Utilities.h"
#include "aeongames/ResourceCache.h"
#include <unordered_map>
namespace AeonGames
{
    static std::unordered_map<std::string, std::function<std::shared_ptr<Renderer> ( const std::string& ) >> RendererLoaders;

    std::shared_ptr<Renderer> GetRenderer ( const std::string& aFilename )
    {
        auto it = RendererLoaders.find ( GetFileExtension ( aFilename ) );
        if ( it != RendererLoaders.end() )
        {
            return it->second ( aFilename );
        }
        return nullptr;
    }
    bool RegisterRendererLoader ( const std::string& aExt, std::function<std::shared_ptr<Renderer> ( const std::string& ) > aLoader )
    {
        if ( RendererLoaders.find ( aExt ) == RendererLoaders.end() )
        {
            RendererLoaders[aExt] = aLoader;
            return true;
        }
        return false;
    }
    bool UnregisterRendererLoader ( const std::string& aExt )
    {
        auto it = RendererLoaders.find ( aExt );
        if ( it != RendererLoaders.end() )
        {
            RendererLoaders.erase ( it );
            return true;
        }
        return false;
    }
}

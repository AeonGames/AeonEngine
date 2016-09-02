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
#include "aeongames/Image.h"
#include "aeongames/Utilities.h"
#include "aeongames/ResourceCache.h"
#include <unordered_map>
namespace AeonGames
{
    static std::unordered_map<std::string, std::function<std::unique_ptr<Image> ( const std::string& ) >> ImageLoaders;

    std::shared_ptr<Image> GetImage ( const std::string& aFilename )
    {
        auto it = ImageLoaders.find ( GetFileExtension ( aFilename ) );
        if ( it != ImageLoaders.end() )
        {
            return it->second ( aFilename );
        }
        return nullptr;
    }
    bool RegisterImageLoader ( const std::string& aExt, std::function<std::unique_ptr<Image> ( const std::string& ) > aLoader )
    {
        if ( ImageLoaders.find ( aExt ) == ImageLoaders.end() )
        {
            ImageLoaders[aExt] = aLoader;
            return true;
        }
        return false;
    }
    bool UnregisterImageLoader ( const std::string& aExt )
    {
        auto it = ImageLoaders.find ( aExt );
        if ( it != ImageLoaders.end() )
        {
            ImageLoaders.erase ( it );
            return true;
        }
        return false;
    }
}

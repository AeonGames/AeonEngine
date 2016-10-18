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
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <utility>
namespace AeonGames
{
    /**@todo unique_ptrs should really be unique_ptrs.*/
    template<class T, typename... Args>
    class Factory
    {
    public:
        static std::unique_ptr<T> Get ( const std::string& aIdentifier, Args&&... args )
        {
            auto it = Loaders.find ( aIdentifier );
            if ( it != Loaders.end() )
            {
                return it->second ( std::forward<Args> ( args )... );
            }
            return nullptr;
        }
        static bool RegisterLoader ( const std::string& aIdentifier, std::function < std::unique_ptr<T> ( Args&&... args ) > aLoader )
        {
            if ( Loaders.find ( aIdentifier ) == Loaders.end() )
            {
                Loaders[aIdentifier] = aLoader;
                return true;
            }
            return false;
        }
        static bool UnregisterLoader ( const std::string& aIdentifier )
        {
            auto it = Loaders.find ( aIdentifier );
            if ( it != Loaders.end() )
            {
                Loaders.erase ( it );
                return true;
            }
            return false;
        }
    private:
        static std::unordered_map < std::string, std::function < std::unique_ptr<T> ( Args&&... args ) >> Loaders;
    };
    template<class T, typename... Args>
    std::unordered_map < std::string, std::function < std::unique_ptr<T> ( Args&&... args ) >> Factory<T, Args...>::Loaders;
}

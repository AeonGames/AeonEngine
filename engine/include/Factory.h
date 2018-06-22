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
#include <unordered_map>
#include <string>
#include <functional>
#include "aeongames/Memory.h"
#include <utility>
namespace AeonGames
{
    template<class T, typename... Args>
    class Factory
    {
    public:
        static std::unique_ptr<T> Construct ( const std::string& aIdentifier, Args&&... args )
        {
            auto it = Constructors.find ( aIdentifier );
            if ( it != Constructors.end() )
            {
                return it->second ( std::forward<Args> ( args )... );
            }
            return nullptr;
        }
        static bool RegisterConstructor ( const std::string& aIdentifier, const std::function < std::unique_ptr<T> ( Args&&... args ) > & aConstructor )
        {
            if ( Constructors.find ( aIdentifier ) == Constructors.end() )
            {
                Constructors[aIdentifier] = aConstructor;
                return true;
            }
            return false;
        }
        static bool UnregisterConstructor ( const std::string& aIdentifier )
        {
            auto it = Constructors.find ( aIdentifier );
            if ( it != Constructors.end() )
            {
                Constructors.erase ( it );
                return true;
            }
            return false;
        }
        static void EnumerateConstructors ( const std::function<bool ( const std::string& ) >& aEnumerator )
        {
            for ( auto& i : Constructors )
            {
                if ( !aEnumerator ( i.first ) )
                {
                    return;
                }
            }
        }
    private:
        static std::unordered_map < std::string, std::function < std::unique_ptr<T> ( Args&&... args ) >> Constructors;
    };
    template<class T, typename... Args>
    std::unordered_map < std::string, std::function < std::unique_ptr<T> ( Args&&... args ) >> Factory<T, Args...>::Constructors;
}

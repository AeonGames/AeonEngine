/*
Copyright (C) 2016-2020 Rodrigo Jose Hernandez Cordoba

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
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <utility>
#include <tuple>
#include <algorithm>
#include "aeongames/StringId.h"

#define FactoryDefinition(X) \
    std::unique_ptr<X> Construct##X ( uint32_t aIdentifier );\
    std::unique_ptr<X> Construct##X ( const std::string& aIdentifier );\
    std::unique_ptr<X> Construct##X ( const StringId& aIdentifier );\
    bool Register##X##Constructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<X>() >& aConstructor ); \
    bool Unregister##X##Constructor ( const StringId& aIdentifier );\
    void Enumerate##X##Constructors ( const std::function<bool ( const StringId& ) >& aEnumerator ); \
    void std::vector<std::string> Get##X##ConstructorNames();

#define FactoryImplementation(X) \
    std::unique_ptr<X> Construct##X ( uint32_t aIdentifier )\
    { \
        return Factory<X>::Construct ( aIdentifier ); \
    } \
    std::unique_ptr<X> Construct##X ( const std::string& aIdentifier )\
    { \
        return Factory<X>::Construct ( aIdentifier ); \
    } \
    std::unique_ptr<X> Construct##X ( const StringId& aIdentifier )\
    { \
        return Factory<X>::Construct ( aIdentifier.GetId() ); \
    } \
    bool Register##X##Constructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<X>() >& aConstructor ) \
    { \
        return Factory<X>::RegisterConstructor ( aIdentifier, aConstructor );\
    }\
    bool Unregister##X##Constructor ( const StringId& aIdentifier )\
    {\
        return Factory<X>::UnregisterConstructor ( aIdentifier );\
    }\
    void Enumerate##X##Constructors ( const std::function<bool ( const StringId& ) >& aEnumerator )\
    {\
        Factory<X>::EnumerateConstructors ( aEnumerator );\
    }\
    std::vector<std::string> Get##X##ConstructorNames()\
    {\
        return Factory<X>::GetConstructorNames();\
    }

namespace AeonGames
{
    template<class T>
    class Factory
    {
    public:
        using Constructor = std::tuple<StringId, std::function < std::unique_ptr<T>() >>;
        static std::unique_ptr<T> Construct ( uint32_t aIdentifier )
        {
            auto it = std::find_if ( Constructors.begin(), Constructors.end(),
                                     [aIdentifier] ( const Constructor & aConstructor )
            {
                return aIdentifier == std::get<0> ( aConstructor );
            } );
            if ( it != Constructors.end() )
            {
                return std::get<1> ( *it ) ();
            }
            return nullptr;
        }
        static std::unique_ptr<T> Construct ( const std::string& aIdentifier )
        {
            return Construct ( crc32i ( aIdentifier.data(), aIdentifier.size() ) );
        }
        static bool RegisterConstructor ( const StringId& aIdentifier, const std::function < std::unique_ptr<T>() > & aConstructor )
        {
            auto it = std::find_if ( Constructors.begin(), Constructors.end(),
                                     [aIdentifier] ( const Constructor & aConstructor )
            {
                return aIdentifier == std::get<0> ( aConstructor );
            } );
            if ( it == Constructors.end() )
            {
                Constructors.emplace_back ( aIdentifier, aConstructor );
                return true;
            }
            return false;
        }
        static bool UnregisterConstructor ( const StringId& aIdentifier )
        {
            auto it = std::find_if ( Constructors.begin(), Constructors.end(),
                                     [aIdentifier] ( const Constructor & aConstructor )
            {
                return aIdentifier == std::get<0> ( aConstructor );
            } );
            if ( it != Constructors.end() )
            {
                Constructors.erase ( it );
                return true;
            }
            return false;
        }
        static void EnumerateConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator )
        {
            for ( auto& i : Constructors )
            {
                if ( !aEnumerator ( std::get<0> ( i ) ) )
                {
                    return;
                }
            }
        }

        static std::vector<std::string> GetConstructorNames()
        {
            std::vector<std::string> names{Constructors.size() };
            std::transform ( Constructors.begin(), Constructors.end(), names.begin(),
                             [] ( const Constructor & constructor )
            {
                return std::get<0> ( constructor ).GetString();
            } );
            return names;
        }

    private:
        static std::vector < Constructor > Constructors;
    };
    template<class T>
    std::vector<std::tuple<StringId, std::function < std::unique_ptr<T>() >>> Factory<T>::Constructors;
}

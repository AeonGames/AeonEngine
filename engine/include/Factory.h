/*
Copyright (C) 2016-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/StringId.hpp"

/** @brief Macro that declares factory functions for a given type.
    @param X The type to create factory functions for. */
#define FactoryDefinition(X,...) \
    std::unique_ptr<X> Construct##X ( uint32_t aIdentifier,##__VA_ARGS__);\
    std::unique_ptr<X> Construct##X ( const std::string& aIdentifier,##__VA_ARGS__ );\
    std::unique_ptr<X> Construct##X ( const StringId& aIdentifier,##__VA_ARGS__ );\
    bool Register##X##Constructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<X>(__VA_ARGS__) >& aConstructor ); \
    bool Unregister##X##Constructor ( const StringId& aIdentifier );\
    void Enumerate##X##Constructors ( const std::function<bool ( const StringId& ) >& aEnumerator ); \
    void std::vector<std::string> Get##X##ConstructorNames();

/** @brief Macro that implements factory functions for a zero-argument constructor type.
    @param X The type to implement factory functions for. */
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

/** @brief Macro that implements factory functions for a single-argument constructor type.
    @param X The type to implement factory functions for.
    @param Y The argument type for the constructor. */
#define FactoryImplementation1Arg(X,Y) \
    std::unique_ptr<X> Construct##X ( uint32_t aIdentifier, Y ARG )\
    { \
        return Factory<X,Y>::Construct ( aIdentifier, ARG ); \
    } \
    std::unique_ptr<X> Construct##X ( const std::string& aIdentifier, Y ARG )\
    { \
        return Factory<X,Y>::Construct ( aIdentifier, ARG ); \
    } \
    std::unique_ptr<X> Construct##X ( const StringId& aIdentifier, Y ARG )\
    { \
        return Factory<X,Y>::Construct ( aIdentifier.GetId(), ARG ); \
    } \
    bool Register##X##Constructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<X>(Y) >& aConstructor ) \
    { \
        return Factory<X,Y>::RegisterConstructor ( aIdentifier, aConstructor );\
    }\
    bool Unregister##X##Constructor ( const StringId& aIdentifier )\
    {\
        return Factory<X,Y>::UnregisterConstructor ( aIdentifier );\
    }\
    void Enumerate##X##Constructors ( const std::function<bool ( const StringId& ) >& aEnumerator )\
    {\
        Factory<X,Y>::EnumerateConstructors ( aEnumerator );\
    }\
    std::vector<std::string> Get##X##ConstructorNames()\
    {\
        return Factory<X,Y>::GetConstructorNames();\
    }

namespace AeonGames
{
    /** @brief Generic factory that registers and invokes named constructors for a given type.
        @tparam T The base type produced by the factory.
        @tparam Types Additional argument types forwarded to the constructor. */
    template<class T, typename... Types>
    class Factory
    {
    public:
        /** @brief Tuple pairing a StringId identifier with a constructor function. */
        using Constructor = std::tuple<StringId, std::function < std::unique_ptr<T> ( Types... args ) >>;
        /** @brief Constructs an object by its numeric identifier.
            @param aIdentifier CRC32 identifier.
            @param args Additional arguments forwarded to the constructor.
            @return A unique_ptr to the constructed object, or nullptr if not found. */
        static std::unique_ptr<T> Construct ( uint32_t aIdentifier, Types... args )
        {
            auto it = std::find_if ( Constructors.begin(), Constructors.end(),
                                     [aIdentifier] ( const Constructor & aConstructor )
            {
                return aIdentifier == std::get<0> ( aConstructor );
            } );
            if ( it != Constructors.end() )
            {
                return std::get<1> ( *it ) ( args... );
            }
            return nullptr;
        }
        /** @brief Constructs an object by its string identifier.
            @param aIdentifier String identifier (hashed to CRC32 internally).
            @param args Additional arguments forwarded to the constructor.
            @return A unique_ptr to the constructed object, or nullptr if not found. */
        static std::unique_ptr<T> Construct ( const std::string& aIdentifier, Types... args )
        {
            return Construct ( crc32i ( aIdentifier.data(), aIdentifier.size() ), args... );
        }
        /** @brief Registers a constructor function with the given identifier.
            @param aIdentifier Identifier for the constructor.
            @param aConstructor Function that creates instances of T.
            @return true if registration succeeded, false if the identifier is already registered. */
        static bool RegisterConstructor ( const StringId& aIdentifier, const std::function < std::unique_ptr<T> ( Types... args ) > & aConstructor )
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
        /** @brief Unregisters the constructor with the given identifier.
            @param aIdentifier Identifier of the constructor to remove.
            @return true if the constructor was found and removed, false otherwise. */
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
        /** @brief Enumerates all registered constructors, calling the provided function for each.
            @param aEnumerator Callback receiving a StringId; return false to stop enumeration. */
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

        /** @brief Returns the names of all registered constructors. */
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
        /// @brief Registered factory constructors.
        static std::vector < Constructor > Constructors;
    };
    /// @cond INTERNAL
    template<class T, typename... Types>
    std::vector<typename Factory<T, Types...>::Constructor> Factory<T, Types...>::Constructors;
    /// @endcond

    /** @brief Free function template that constructs an object by numeric identifier.
        @tparam T The type to construct.
        @tparam Types Additional argument types.
        @param aIdentifier CRC32 identifier.
        @param args Constructor arguments.
        @return A unique_ptr to the constructed object, or nullptr if not found. */
    template<class T, typename... Types>
    std::unique_ptr<T> Construct ( uint32_t aIdentifier, Types... args )
    {
        return Factory<T, Types...>::Construct ( aIdentifier, args... );
    }

    /** @brief Free function template that constructs an object by string identifier. */
    template<class T, typename... Types>
    std::unique_ptr<T> Construct ( const std::string& aIdentifier, Types... args )
    {
        return Factory<T, Types...>::Construct ( aIdentifier, args... );
    }

    /** @brief Free function template that constructs an object by StringId. */
    template<class T, typename... Types>
    std::unique_ptr<T> Construct ( const StringId& aIdentifier, Types... args )
    {
        return Factory<T, Types...>::Construct ( aIdentifier.GetId(), args... );
    }

    /** @brief Free function template that registers a constructor. */
    template<class T, typename... Types>
    bool RegisterConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<T> ( Types... args ) >& aConstructor )
    {
        return Factory<T, Types...>::RegisterConstructor ( aIdentifier, aConstructor );
    }

    /** @brief Free function template that unregisters a constructor. */
    template<class T, typename... Types>
    bool UnregisterConstructor ( const StringId& aIdentifier )
    {
        return Factory<T, Types...>::UnregisterConstructor ( aIdentifier );
    }

    /** @brief Free function template that enumerates all registered constructors. */
    template<class T, typename... Types>
    void EnumerateConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator )
    {
        Factory<T, Types...>::EnumerateConstructors ( aEnumerator );
    }

    /** @brief Free function template that returns all registered constructor names. */
    template<class T, typename... Types>
    std::vector<std::string> GetConstructorNames()
    {
        return Factory<T, Types...>::GetConstructorNames();
    }
}

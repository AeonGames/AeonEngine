/*
Copyright (C) 2018,2022,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <tuple>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <exception>
#include <unordered_map>
#include "aeongames/AeonEngine.hpp"
#include "aeongames/ResourceFactory.hpp"
#include "aeongames/ResourceCache.hpp"
#include "aeongames/ResourceId.hpp"
#include "aeongames/LogLevel.hpp"

namespace AeonGames
{
    static std::unordered_map < uint32_t, std::tuple<std::function < UniqueAnyPtr ( uint32_t ) >, UniqueAnyPtr >> Constructors;

    /** @brief Format a CRC32-identified resource component (type or path) for
        diagnostics, including its human readable name when one is known.
        @param crc       The CRC32 hash.
        @param aRegistered Whether a string was explicitly registered for the crc.
        @param aString   The recovered string (may be empty).
        @return A string such as 'Model (0x16545ddd)', '"sponza/foo.txt" (0x...)',
                '<empty> (0x00000000)' or '<unknown> (0x...)'. */
    static std::string DescribeResourceComponent ( uint32_t crc, bool aRegistered, const std::string& aString )
    {
        std::ostringstream stream;
        if ( aRegistered )
        {
            if ( aString.empty() )
            {
                stream << "<empty>";
            }
            else
            {
                stream << '"' << aString << '"';
            }
        }
        else
        {
            stream << "<unknown>";
        }
        stream << " (0x" << std::hex << std::setw ( 8 ) << std::setfill ( '0' ) << crc << ")";
        return stream.str();
    }

    UniqueAnyPtr ConstructResource ( const ResourceId& aResourceId )
    {
        const uint32_t type_crc = aResourceId.GetType();
        const uint32_t path_crc = aResourceId.GetPath();
        const std::string type_desc =
            DescribeResourceComponent ( type_crc, HasResourceString ( type_crc ), GetResourceString ( type_crc ) );
        const std::string path_desc =
            DescribeResourceComponent ( path_crc, HasResourceString ( path_crc ), GetResourcePath ( path_crc ) );

        auto it = Constructors.find ( type_crc );
        if ( it != Constructors.end() )
        {
            // Debug log of the actual strings being resolved, so a failing load
            // can be traced to the exact type and path that were requested.
            std::cout << LogLevel::Info << "Constructing resource: type " << type_desc
                      << ", path " << path_desc << std::endl;
            try
            {
                return std::get<0> ( it->second ) ( path_crc );
            }
            catch ( const std::exception& e )
            {
                std::ostringstream stream;
                stream << "Failed to construct resource (type " << type_desc
                       << ", path " << path_desc << "): " << e.what();
                throw std::runtime_error ( stream.str() );
            }
        }
        std::ostringstream stream;
        stream << "No constructor registered for type " << type_desc
               << " (resource path " << path_desc << ")";
        throw std::runtime_error ( stream.str().c_str() );
    }
    bool RegisterResourceConstructor ( uint32_t aType, const std::function < UniqueAnyPtr ( uint32_t ) > & aConstructor, UniqueAnyPtr&& aDefaultResource )
    {
        if ( Constructors.find ( aType ) == Constructors.end() )
        {
            Constructors[aType] = std::make_pair ( aConstructor, std::move ( aDefaultResource ) );
            return true;
        }
        return false;
    }
    bool UnregisterResourceConstructor ( uint32_t aType )
    {
        auto it = Constructors.find ( aType );
        if ( it != Constructors.end() )
        {
            Constructors.erase ( it );
            return true;
        }
        return false;
    }
    void EnumerateResourceConstructors ( const std::function<bool ( uint32_t ) >& aEnumerator )
    {
        for ( auto& i : Constructors )
        {
            if ( !aEnumerator ( i.first ) )
            {
                return;
            }
        }
    }

    const UniqueAnyPtr& GetDefaultResource ( uint32_t aType )
    {
        static const UniqueAnyPtr unique_any_nullptr{nullptr};
        auto it = Constructors.find ( aType );
        if ( it != Constructors.end() )
        {
            return std::get<1> ( it->second );
        }
        return unique_any_nullptr;
    }
}

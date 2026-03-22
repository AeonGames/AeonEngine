/*
Copyright (C) 2018,2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_RESOURCEID_H
#define AEONGAMES_RESOURCEID_H

#include <cstdint>
#include <string>
#include "aeongames/AeonEngine.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/ResourceFactory.hpp"
#include "aeongames/ResourceCache.hpp"

namespace AeonGames
{
    /** @brief Identifies a resource by its type and path CRC32 hashes. */
    class ResourceId
    {
    public:
        /** @brief Default constructor. Creates an empty resource identifier. */
        ResourceId() = default;
        /** @brief Copy constructor. */
        ResourceId ( const ResourceId& ) = default;
        /** @brief Construct from pre-computed CRC32 hashes.
         *  @param aType CRC32 hash of the resource type.
         *  @param aPath CRC32 hash of the resource path. */
        ResourceId ( uint32_t aType, uint32_t aPath ) :
            mType{aType}, mPath{aPath} {}
        /** @brief Construct from type and path strings (hashed internally).
         *  @param aType Resource type string.
         *  @param aPath Resource path string. */
        ResourceId ( const std::string& aType, const std::string& aPath ) :
            mType{crc32i ( aType.data(), aType.length() ) }, mPath{crc32i ( aPath.data(), aPath.length() ) } {}
        /** @brief Construct from a type string and a pre-computed path hash.
         *  @param aType Resource type string.
         *  @param aPath CRC32 hash of the resource path. */
        ResourceId ( const std::string& aType, uint32_t aPath ) :
            mType{crc32i ( aType.data(), aType.length() ) }, mPath{aPath} {}
        /** @brief Construct from a pre-computed type hash and a path string.
         *  @param aType CRC32 hash of the resource type.
         *  @param aPath Resource path string. */
        ResourceId ( uint32_t aType, const std::string& aPath ) :
            mType{aType}, mPath{crc32i ( aPath.data(), aPath.length() ) } {}
        ~ResourceId() = default;

        /** @brief Check whether the identified resource is currently loaded.
         *  @return True if the resource exists in the cache. */
        operator bool() const
        {
            return GetResource ( *this ).GetRaw() != nullptr;
        }

        /** @brief Get the CRC32 hash of the resource type.
         *  @return Type hash. */
        uint32_t GetType() const
        {
            return mType;
        }

        /** @brief Get the CRC32 hash of the resource path.
         *  @return Path hash. */
        uint32_t GetPath() const
        {
            return mPath;
        }

        /** @brief Get the original resource path string from the hash.
         *  @return Resource path string. */
        std::string GetPathString() const
        {
            return GetResourcePath ( mPath );
        }

        /** @brief Cast the cached resource to the specified type without loading.
         *  @tparam T Target resource type.
         *  @return Pointer to the resource, or nullptr if not cached or wrong type. */
        template<typename T>
        T* Cast() const
        {
            return GetResource ( *this ).Get<T>();
        }

        /** @brief Get the resource, loading and caching it if necessary.
         *  @tparam T Target resource type.
         *  @return Pointer to the resource. */
        template<typename T>
        T* Get() const
        {
            T* t = GetResource ( *this ).Get<T>();
            if ( !t )
            {
                t = StoreResource ( mPath, ConstructResource ( *this ) ).Get<T>();
            }
            return t;
        }

        /** @brief Construct and store the resource in the cache if not already present. */
        void Store() const
        {
            // Don't store nullptrs
            if ( !GetResource ( *this ).GetRaw() )
            {
                StoreResource ( mPath, ConstructResource ( *this ) );
            }
        }

        /** @brief Remove the resource from the cache if present. */
        void Dispose() const
        {
            if ( GetResource ( *this ).GetRaw() )
            {
                DisposeResource ( mPath );
            }
        }

    private:
        uint32_t mType{};
        uint32_t mPath{};
    };
}

#endif

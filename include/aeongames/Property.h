/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_PROPERTY_H
#define AEONGAMES_PROPERTY_H
#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include "aeongames/CRC.h"

namespace AeonGames
{
    template<class T>
    class Property
    {
    public:
        Property (
            const std::string& aName,
            const std::string& aDisplayName,
            const std::string& aFormat,
            const std::function<void ( T*, const void* ) >& aSetter,
            const std::function<void ( const T*, void* ) >& aGetter,
            std::initializer_list<Property> aSubProperties = {}
        ) :
            mId{crc32i ( aName.c_str(), aName.size() ) },
            mName{aName},
            mDisplayName{aDisplayName},
            mFormat{aFormat},
            mSetter{aSetter},
            mGetter{aGetter},
            mParent{},
            mSubProperties{std::move ( aSubProperties ) }
        {
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
        }
        Property ( const Property& aProperty ) :
            mId{aProperty.mId},
            mName{aProperty.mName},
            mDisplayName{aProperty.mDisplayName},
            mFormat{aProperty.mFormat},
            mSetter{aProperty.mSetter},
            mGetter{aProperty.mGetter},
            mParent{aProperty.mParent},
            mSubProperties{ aProperty.mSubProperties }
        {
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
        }
        Property& operator= ( const Property& aProperty )
        {
            mId = aProperty.mId;
            mName = aProperty.mName;
            mDisplayName = aProperty.mDisplayName;
            mFormat = aProperty.mFormat;
            mSetter = aProperty.mSetter;
            mGetter = aProperty.mGetter;
            mParent = aProperty.mParent;
            mSubProperties = aProperty.mSubProperties;
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
            return *this;
        }
        Property ( const Property&& aProperty ) :
            mId{aProperty.mId},
            mName{std::move ( aProperty.mName ) },
            mDisplayName{std::move ( aProperty.mDisplayName ) },
            mFormat{std::move ( aProperty.mFormat ) },
            mSetter{std::move ( aProperty.mSetter ) },
            mGetter{std::move ( aProperty.mGetter ) },
            mParent{aProperty.mParent},
            mSubProperties{ std::move ( aProperty.mSubProperties ) }
        {
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
        }
        Property& operator= ( const Property&& aProperty )
        {
            mId = aProperty.mId;
            mName = std::move ( aProperty.mName );
            mDisplayName = std::move ( aProperty.mDisplayName );
            mFormat = std::move ( aProperty.mFormat );
            mParent = aProperty.mParent;
            mSubProperties = std::move ( aProperty.mSubProperties );
            for ( auto& i : mSubProperties )
            {
                i.mParent = this;
            }
            return *this;
        }
        const size_t GetId() const
        {
            return mId;
        }
        const std::string& GetName() const
        {
            return mName;
        }
        const std::string& GetDisplayName() const
        {
            return mDisplayName;
        }
        const std::string& GetFormat() const
        {
            return mFormat;
        }
        void Set ( T* aInstance, const void* aTuple ) const
        {
            mSetter ( aInstance, aTuple );
        }
        void Get ( const T* aInstance, void* aTuple ) const
        {
            mGetter ( aInstance, aTuple );
        }
        const Property* GetParent() const
        {
            return mParent;
        }
        const std::vector<Property>& GetSubProperties() const
        {
            return mSubProperties;
        }
        size_t GetIndex() const
        {
            auto parent = mParent;
            if ( parent )
            {
                auto index = std::find_if ( parent->GetSubProperties().begin(), parent->GetSubProperties().end(),
                                            [this] ( const Property & property )
                {
                    return &property == this;
                } );
                return index - parent->GetSubProperties().begin();
            }
            throw std::runtime_error ( "Property has no parent and thus no assigned index." );
        }
    private:
        size_t mId {};
        std::string  mName{};
        std::string  mDisplayName{};
        std::string  mFormat{};
        std::function<void ( T*, const void* ) > mSetter;
        std::function<void ( const T*, void* ) > mGetter;
        Property* mParent{};
        std::vector<Property> mSubProperties{};
    };
}
#endif

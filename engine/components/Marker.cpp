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

#include <array>
#include "Marker.h"

namespace AeonGames
{
    static const StringId MarkerStringId{"Marker"};
    const StringId& Marker::GetClassId()
    {
        return MarkerStringId;
    }

    Marker::Marker() : Component{} {}
    Marker::~Marker() = default;

    const StringId& Marker::GetId() const
    {
        return MarkerStringId;
    }

    static constexpr std::array<const StringId, 1> MarkerPropertyIds
    {
        {
            {"Type"}
        }
    };

    size_t Marker::GetPropertyCount () const
    {
        return MarkerPropertyIds.size();
    }

    const StringId* Marker::GetPropertyInfoArray () const
    {
        return MarkerPropertyIds.data();
    }

    Property Marker::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case MarkerPropertyIds[0]:
            return mType;
        }
        return Property{};
    }

    void Marker::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        if ( !std::holds_alternative<std::string> ( aProperty ) )
        {
            return;
        }
        switch ( aId )
        {
        case MarkerPropertyIds[0]:
            mType = std::get<std::string> ( aProperty );
            break;
        }
    }

    void Marker::Update ( Node& /*aNode*/, double /*aDelta*/ )
    {
        // A marker only has to exist; the node's name and transform are the data.
    }

    void Marker::ProcessMessage ( Node& /*aNode*/, uint32_t /*aMessageType*/, const void* /*aMessageData*/ )
    {
    }
}

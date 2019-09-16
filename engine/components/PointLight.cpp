/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <cmath>
#include "PointLight.h"
#include "aeongames/AeonEngine.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/Window.h"
#include "aeongames/Buffer.h"
#include "aeongames/Renderer.h"
#include "aeongames/Node.h"

namespace AeonGames
{
    static const StringId PointLightStringId{"Point Light"};
    const StringId& PointLight::GetClassId()
    {
        return PointLightStringId;
    }

    PointLight::PointLight() : Component{}
    {
    }

    PointLight::~PointLight() = default;

    const StringId& PointLight::GetId() const
    {
        return PointLightStringId;
    }

    static constexpr std::array<const StringId, 0> PointLightPropertyIds{};

    size_t PointLight::GetPropertyCount () const
    {
        return PointLightPropertyIds.size();
    }

    const StringId* PointLight::GetPropertyInfoArray () const
    {
        return PointLightPropertyIds.data();
    }

    Property PointLight::GetProperty ( const StringId& aId ) const
    {
        return Property{};
    }

    void PointLight::SetProperty ( uint32_t aId, const Property& aProperty )
    {
    }

    void PointLight::Update ( Node& aNode, double aDelta, Window* aWindow )
    {
    }

    void PointLight::Render ( const Node& aNode, const Window& aWindow ) const
    {
    }

    void PointLight::ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData )
    {
    }
}

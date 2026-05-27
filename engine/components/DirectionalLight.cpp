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
#include "DirectionalLight.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/GpuLight.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Quaternion.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Vector3.hpp"

namespace AeonGames
{
    static const StringId DirectionalLightStringId{"Directional Light"};
    const StringId& DirectionalLight::GetClassId()
    {
        return DirectionalLightStringId;
    }

    DirectionalLight::DirectionalLight() : Component{} {}
    DirectionalLight::~DirectionalLight() = default;

    const StringId& DirectionalLight::GetId() const
    {
        return DirectionalLightStringId;
    }

    static constexpr std::array<const StringId, 4> DirectionalLightPropertyIds
    {
        {
            {"Color R"},
            {"Color G"},
            {"Color B"},
            {"Intensity"}
        }
    };

    size_t DirectionalLight::GetPropertyCount () const
    {
        return DirectionalLightPropertyIds.size();
    }

    const StringId* DirectionalLight::GetPropertyInfoArray () const
    {
        return DirectionalLightPropertyIds.data();
    }

    Property DirectionalLight::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case DirectionalLightPropertyIds[0]:
            return mColorR;
        case DirectionalLightPropertyIds[1]:
            return mColorG;
        case DirectionalLightPropertyIds[2]:
            return mColorB;
        case DirectionalLightPropertyIds[3]:
            return mIntensity;
        }
        return Property{};
    }

    void DirectionalLight::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        if ( !std::holds_alternative<float> ( aProperty ) )
        {
            return;
        }
        float value = std::get<float> ( aProperty );
        switch ( aId )
        {
        case DirectionalLightPropertyIds[0]:
            mColorR = value;
            break;
        case DirectionalLightPropertyIds[1]:
            mColorG = value;
            break;
        case DirectionalLightPropertyIds[2]:
            mColorB = value;
            break;
        case DirectionalLightPropertyIds[3]:
            mIntensity = value;
            break;
        }
    }

    void DirectionalLight::Update ( Node& aNode, double /*aDelta*/ )
    {
        Scene* scene = aNode.GetScene();
        if ( !scene )
        {
            return;
        }
        // Directional shines along local -Z, so the world-space "to-light"
        // vector is the node's local +Z axis rotated into world space. Position
        // and radius are unused; the shader skips falloff for Directional.
        const Vector3 to_light_world = aNode.GetGlobalTransform().GetRotation() * Vector3{0.0f, 0.0f, 1.0f};

        GpuLight light{};
        light.position_radius    = Vector4{ 0.0f, 0.0f, 0.0f, 0.0f };
        light.color_intensity    = Vector4{ mColorR, mColorG, mColorB, mIntensity };
        light.direction_cosOuter = Vector4{ to_light_world.GetX(), to_light_world.GetY(), to_light_world.GetZ(), -1.0f };
        light.type               = static_cast<uint32_t> ( LightType::Directional );
        scene->AddLight ( light );
    }

    void DirectionalLight::Render ( const Node& /*aNode*/, Renderer& /*aRenderer*/, void* /*aWindowId*/ ) {}
    void DirectionalLight::ProcessMessage ( Node& /*aNode*/, uint32_t /*aMessageType*/, const void* /*aMessageData*/ ) {}
}

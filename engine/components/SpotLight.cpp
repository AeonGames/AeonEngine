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
#include <cmath>
#include "SpotLight.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/GpuLight.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Quaternion.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Vector3.hpp"

namespace AeonGames
{
    static const StringId SpotLightStringId{"Spot Light"};
    const StringId& SpotLight::ClassId()
    {
        return SpotLightStringId;
    }

    SpotLight::SpotLight() = default;
    SpotLight::~SpotLight() = default;

    const StringId& SpotLight::GetClassId() const
    {
        return SpotLightStringId;
    }

    void SpotLight::SetInnerAngle ( float aAngle )
    {
        mInnerAngle = aAngle;
        if ( mInnerAngle > mOuterAngle )
        {
            mInnerAngle = mOuterAngle;
        }
    }

    void SpotLight::SetOuterAngle ( float aAngle )
    {
        mOuterAngle = aAngle;
        if ( mInnerAngle > mOuterAngle )
        {
            mInnerAngle = mOuterAngle;
        }
    }

    void SpotLight::Update ( Node& aNode, double /*aDelta*/ )
    {
        Scene* scene = aNode.GetScene();
        if ( !scene )
        {
            return;
        }
        const Transform& xform = aNode.GetGlobalTransform();
        const Vector3& world_pos = xform.GetTranslation();
        // Spot shines along local -Z, so the world-space "to-light" vector
        // (which the shader compares against the surface-to-light direction)
        // is the node's local +Z axis rotated into world space.
        const Vector3 to_light_world = xform.GetRotation() * Vector3{0.0f, 0.0f, 1.0f};

        constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;
        const float cos_inner = std::cos ( mInnerAngle * kDegToRad );
        const float cos_outer = std::cos ( mOuterAngle * kDegToRad );

        GpuLight light{};
        light.position_radius    = Vector4{ world_pos.GetX(), world_pos.GetY(), world_pos.GetZ(), mRadius };
        light.color_intensity    = Vector4{ mColorR, mColorG, mColorB, mIntensity };
        light.direction_cosOuter = Vector4{ to_light_world.GetX(), to_light_world.GetY(), to_light_world.GetZ(), cos_outer };
        light.type               = static_cast<uint32_t> ( LightType::Spot );
        light.cos_inner          = cos_inner;
        scene->AddLight ( light );
    }

    void SpotLight::ProcessMessage ( Node& /*aNode*/, uint32_t /*aMessageType*/, const void* /*aMessageData*/ ) {}
}

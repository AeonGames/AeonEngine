/*
Copyright (C) 2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Buffer.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Node.hpp"

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

    static constexpr std::array<const StringId, 5> PointLightPropertyIds
    {
        {
            {"Color R"},
            {"Color G"},
            {"Color B"},
            {"Intensity"},
            {"Radius"}
        }
    };

    size_t PointLight::GetPropertyCount () const
    {
        return PointLightPropertyIds.size();
    }

    const StringId* PointLight::GetPropertyInfoArray () const
    {
        return PointLightPropertyIds.data();
    }

    float PointLight::GetColorR()    const
    {
        return mColorR;
    }
    float PointLight::GetColorG()    const
    {
        return mColorG;
    }
    float PointLight::GetColorB()    const
    {
        return mColorB;
    }
    float PointLight::GetIntensity() const
    {
        return mIntensity;
    }
    float PointLight::GetRadius()    const
    {
        return mRadius;
    }
    void  PointLight::SetColorR ( float v )
    {
        mColorR = v;
    }
    void  PointLight::SetColorG ( float v )
    {
        mColorG = v;
    }
    void  PointLight::SetColorB ( float v )
    {
        mColorB = v;
    }
    void  PointLight::SetIntensity ( float v )
    {
        mIntensity = v;
    }
    void  PointLight::SetRadius ( float v )
    {
        mRadius = v;
    }

    Property PointLight::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case PointLightPropertyIds[0]:
            return mColorR;
        case PointLightPropertyIds[1]:
            return mColorG;
        case PointLightPropertyIds[2]:
            return mColorB;
        case PointLightPropertyIds[3]:
            return mIntensity;
        case PointLightPropertyIds[4]:
            return mRadius;
        }
        return Property{};
    }

    void PointLight::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        if ( !std::holds_alternative<float> ( aProperty ) )
        {
            return;
        }
        float value = std::get<float> ( aProperty );
        switch ( aId )
        {
        case PointLightPropertyIds[0]:
            mColorR    = value;
            break;
        case PointLightPropertyIds[1]:
            mColorG    = value;
            break;
        case PointLightPropertyIds[2]:
            mColorB    = value;
            break;
        case PointLightPropertyIds[3]:
            mIntensity = value;
            break;
        case PointLightPropertyIds[4]:
            mRadius    = value;
            break;
        }
    }

    void PointLight::Update ( Node& aNode, double aDelta )
    {
    }

    void PointLight::Render ( const Node& aNode, Renderer& aRenderer, void* aWindowId )
    {
#if 0
        // Publish this light's world-space position and color to the renderer
        // so subsequent draw calls within this frame can read it from the
        // Lights UBO.
        const Vector3& world_pos = aNode.GetGlobalTransform().GetTranslation();
        Vector3 scaled_color{mColorR * mIntensity, mColorG * mIntensity, mColorB * mIntensity};
        aRenderer.SetLight ( aWindowId, world_pos, scaled_color, mRadius );
#endif
    }

    void PointLight::ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData )
    {
    }
}

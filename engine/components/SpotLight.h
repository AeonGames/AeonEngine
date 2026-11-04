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
#ifndef AEONGAMES_SPOTLIGHT_H
#define AEONGAMES_SPOTLIGHT_H
#include "aeongames/ComponentProperties.hpp"

namespace AeonGames
{
    class Node;
    /** @brief Spot light component. Cone aimed along the node's local -Z axis. */
    class SpotLight final : public ComponentProperties<SpotLight>
    {
    public:
        SpotLight();
        ~SpotLight() final;
        const StringId& GetClassId() const final;
        void Update ( Node& aNode, double aDelta ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        static const StringId& ClassId();
    private:
        float mColorR{1.0f};
        float mColorG{1.0f};
        float mColorB{1.0f};
        float mIntensity{1.0f};
        float mRadius{10.0f};
        // Inner / outer cone half-angles, in degrees. Inner must be smaller
        // than outer; the shader does a smoothstep between cos(outer) and
        // cos(inner) for a soft edge.
        float mInnerAngle{20.0f};
        float mOuterAngle{30.0f};
        void SetInnerAngle ( float aAngle );
        void SetOuterAngle ( float aAngle );
        friend class ComponentProperties<SpotLight>;
        inline static constexpr std::array<ComponentPropertyDescriptor<SpotLight>, 7> kPropertyDescriptors
        {
            {
                ComponentPropertyDescriptor<SpotLight>::Member<&SpotLight::mColorR> ( "Color R" ),
                ComponentPropertyDescriptor<SpotLight>::Member<&SpotLight::mColorG> ( "Color G" ),
                ComponentPropertyDescriptor<SpotLight>::Member<&SpotLight::mColorB> ( "Color B" ),
                ComponentPropertyDescriptor<SpotLight>::Member<&SpotLight::mIntensity> ( "Intensity" ),
                ComponentPropertyDescriptor<SpotLight>::Member<&SpotLight::mRadius> ( "Radius" ),
                ComponentPropertyDescriptor<SpotLight>::Create<&SpotLight::mInnerAngle, &SpotLight::SetInnerAngle> ( "Inner Angle" ),
                ComponentPropertyDescriptor<SpotLight>::Create<&SpotLight::mOuterAngle, &SpotLight::SetOuterAngle> ( "Outer Angle" )
            }};
    };
}
#endif

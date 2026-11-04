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
#ifndef AEONGAMES_DIRECTIONALLIGHT_H
#define AEONGAMES_DIRECTIONALLIGHT_H
#include "aeongames/ComponentProperties.hpp"

namespace AeonGames
{
    class Node;
    /** @brief Directional light component. Shines along the node's local -Z axis,
     *  with no positional falloff (sun-like). */
    class DirectionalLight final : public ComponentProperties<DirectionalLight>
    {
    public:
        DirectionalLight();
        ~DirectionalLight() final;
        const StringId& GetClassId() const final;
        void Update ( Node& aNode, double aDelta ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        static const StringId& ClassId();
    private:
        float mColorR{1.0f};
        float mColorG{1.0f};
        float mColorB{1.0f};
        float mIntensity{1.0f};
        friend class ComponentProperties<DirectionalLight>;
        inline static constexpr std::array<ComponentPropertyDescriptor<DirectionalLight>, 4> kPropertyDescriptors
        {
            {
                ComponentPropertyDescriptor<DirectionalLight>::Member<&DirectionalLight::mColorR> ( "Color R" ),
                ComponentPropertyDescriptor<DirectionalLight>::Member<&DirectionalLight::mColorG> ( "Color G" ),
                ComponentPropertyDescriptor<DirectionalLight>::Member<&DirectionalLight::mColorB> ( "Color B" ),
                ComponentPropertyDescriptor<DirectionalLight>::Member<&DirectionalLight::mIntensity> ( "Intensity" )
            }};
    };
}
#endif

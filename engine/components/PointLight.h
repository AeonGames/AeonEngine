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
#ifndef AEONGAMES_POINTLIGHT_H
#define AEONGAMES_POINTLIGHT_H
#include "aeongames/ComponentProperties.hpp"
#include "aeongames/Vector3.hpp"

namespace AeonGames
{
    class Node;
    class Window;
    /** @brief Point light component representing an omnidirectional light source. */
    class PointLight final : public ComponentProperties<PointLight>
    {
    public:
        /** @brief Default constructor. */
        PointLight();
        /** @name Overrides */
        ///@{
        ~PointLight() final;
        const StringId& GetClassId() const final;
        void Update ( Node& aNode, double aDelta ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}
        /** @brief Returns the class identifier for the PointLight component. */
        static const StringId& ClassId();

        float GetColorR() const;
        float GetColorG() const;
        float GetColorB() const;
        float GetIntensity() const;
        float GetRadius() const;
        void  SetColorR ( float aR );
        void  SetColorG ( float aG );
        void  SetColorB ( float aB );
        void  SetIntensity ( float aIntensity );
        void  SetRadius ( float aRadius );
    private:
        friend class ComponentProperties<PointLight>;
        inline static constexpr std::array<ComponentPropertyDescriptor<PointLight>, 5> kPropertyDescriptors
        {
            {
                ComponentPropertyDescriptor<PointLight>::Create<&PointLight::GetColorR, &PointLight::SetColorR> ( "Color R" ),
                ComponentPropertyDescriptor<PointLight>::Create<&PointLight::GetColorG, &PointLight::SetColorG> ( "Color G" ),
                ComponentPropertyDescriptor<PointLight>::Create<&PointLight::GetColorB, &PointLight::SetColorB> ( "Color B" ),
                ComponentPropertyDescriptor<PointLight>::Create<&PointLight::GetIntensity, &PointLight::SetIntensity> ( "Intensity" ),
                ComponentPropertyDescriptor<PointLight>::Create<&PointLight::GetRadius, &PointLight::SetRadius> ( "Radius" )
            }};
        float mColorR{1.0f};
        float mColorG{1.0f};
        float mColorB{1.0f};
        float mIntensity{1.0f};
        float mRadius{10.0f};
    };
}
#endif

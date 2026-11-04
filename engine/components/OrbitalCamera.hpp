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
#ifndef AEONGAMES_ORBITALCAMERA_HPP
#define AEONGAMES_ORBITALCAMERA_HPP
#include "aeongames/ComponentProperties.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Quaternion.hpp"

namespace AeonGames
{
    class Node;
    /** @brief Orbital camera component.
     */
    class OrbitalCamera final : public ComponentProperties<OrbitalCamera>
    {
    public:
        OrbitalCamera();
        /** @name Overrides */
        ///@{
        ~OrbitalCamera() final;
        const StringId& GetClassId() const final;
        void Update ( Node& aNode, double aDelta ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}

        float GetDistance() const;
        float GetAzimuth() const;
        float GetElevation() const;
        /** Vertical offset from the character origin in the node's local
         * frame (positive = up). */
        float GetHeightOffset() const;
        float GetFieldOfView() const;
        float GetNearPlane() const;
        float GetFarPlane() const;

        void SetDistance ( float aDistance );
        void SetAzimuth ( float aAzimuth );
        void SetElevation ( float aElevation );
        void SetHeightOffset ( float aHeightOffset );
        void SetFieldOfView ( float aFieldOfView );
        void SetNearPlane ( float aNearPlane );
        void SetFarPlane ( float aFarPlane );

        static const StringId& ClassId();

    private:
        friend class ComponentProperties<OrbitalCamera>;
        inline static constexpr std::array<ComponentPropertyDescriptor<OrbitalCamera>, 7> kPropertyDescriptors
        {
            {
                ComponentPropertyDescriptor<OrbitalCamera>::Create<&OrbitalCamera::GetDistance, &OrbitalCamera::SetDistance> ( "Distance" ),
                ComponentPropertyDescriptor<OrbitalCamera>::Create<&OrbitalCamera::GetAzimuth, &OrbitalCamera::SetAzimuth> ( "Azimuth" ),
                ComponentPropertyDescriptor<OrbitalCamera>::Create<&OrbitalCamera::GetElevation, &OrbitalCamera::SetElevation> ( "Elevation" ),
                ComponentPropertyDescriptor<OrbitalCamera>::Create<&OrbitalCamera::GetHeightOffset, &OrbitalCamera::SetHeightOffset> ( "Height Offset" ),
                ComponentPropertyDescriptor<OrbitalCamera>::Create<&OrbitalCamera::GetFieldOfView, &OrbitalCamera::SetFieldOfView> ( "Field of View" ),
                ComponentPropertyDescriptor<OrbitalCamera>::Create<&OrbitalCamera::GetNearPlane, &OrbitalCamera::SetNearPlane> ( "Near Plane" ),
                ComponentPropertyDescriptor<OrbitalCamera>::Create<&OrbitalCamera::GetFarPlane, &OrbitalCamera::SetFarPlane> ( "Far Plane" )
            }};
        float mDistance{1.0f};
        float mAzimuth{};
        float mElevation{};
        float mHeightOffset{1.7f};
        float mFieldOfView{60.0f};
        float mNearPlane{1.0f};
        float mFarPlane{16000.0f};
    };
}
#endif

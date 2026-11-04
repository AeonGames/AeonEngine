/*
Copyright (C) 2018,2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
/** \File Implements the interface for the Portaudio plugin.*/
#include "aeongames/Plugin.hpp"
#include "aeongames/Component.hpp"
#include "aeongames/StringId.hpp"
#include "ModelComponent.h"
#include "Camera.h"
#include "Marker.h"
#include "OverTheShoulderCamera.hpp"
#include "OrbitalCamera.hpp"
#include "CharacterController.hpp"
#include "FreeCamera.hpp"
#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLight.h"
#include "CollisionComponent.hpp"
#include <iostream>

extern "C"
{
    bool AeonEngineComponentsStartUp()
    {
        AeonGames::RegisterComponentConstructor ( AeonGames::ModelComponent::ClassId(), []()
        {
            return std::make_unique<AeonGames::ModelComponent>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::Camera::ClassId(), []()
        {
            return std::make_unique<AeonGames::Camera>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::Marker::ClassId(), []()
        {
            return std::make_unique<AeonGames::Marker>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::OverTheShoulderCamera::ClassId(), []()
        {
            return std::make_unique<AeonGames::OverTheShoulderCamera>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::OrbitalCamera::ClassId(), []()
        {
            return std::make_unique<AeonGames::OrbitalCamera>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::CharacterController::ClassId(), []()
        {
            return std::make_unique<AeonGames::CharacterController>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::FreeCamera::ClassId(), []()
        {
            return std::make_unique<AeonGames::FreeCamera>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::PointLight::ClassId(), []()
        {
            return std::make_unique<AeonGames::PointLight>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::SpotLight::ClassId(), []()
        {
            return std::make_unique<AeonGames::SpotLight>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::DirectionalLight::ClassId(), []()
        {
            return std::make_unique<AeonGames::DirectionalLight>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::CollisionComponent::ClassId(), []()
        {
            return std::make_unique<AeonGames::CollisionComponent>();
        } );
        return true;
    }

    void AeonEngineComponentsShutdown()
    {
        AeonGames::UnregisterComponentConstructor ( AeonGames::ModelComponent::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::Camera::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::Marker::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::OverTheShoulderCamera::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::OrbitalCamera::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::CharacterController::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::FreeCamera::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::PointLight::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::SpotLight::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::DirectionalLight::ClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::CollisionComponent::ClassId() );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "AeonEngine Components",
        "Default Component Implementations",
        AeonEngineComponentsStartUp,
        AeonEngineComponentsShutdown
    };
}

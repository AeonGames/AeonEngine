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
#include "OverTheShoulderCamera.hpp"
#include "CharacterController.hpp"
#include "FreeCamera.hpp"
#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLight.h"
#include <iostream>

extern "C"
{
    bool AeonEngineComponentsStartUp()
    {
        AeonGames::RegisterComponentConstructor ( AeonGames::ModelComponent::GetClassId(), []()
        {
            return std::make_unique<AeonGames::ModelComponent>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::Camera::GetClassId(), []()
        {
            return std::make_unique<AeonGames::Camera>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::OverTheShoulderCamera::GetClassId(), []()
        {
            return std::make_unique<AeonGames::OverTheShoulderCamera>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::CharacterController::GetClassId(), []()
        {
            return std::make_unique<AeonGames::CharacterController>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::FreeCamera::GetClassId(), []()
        {
            return std::make_unique<AeonGames::FreeCamera>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::PointLight::GetClassId(), []()
        {
            return std::make_unique<AeonGames::PointLight>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::SpotLight::GetClassId(), []()
        {
            return std::make_unique<AeonGames::SpotLight>();
        } );
        AeonGames::RegisterComponentConstructor ( AeonGames::DirectionalLight::GetClassId(), []()
        {
            return std::make_unique<AeonGames::DirectionalLight>();
        } );
        return true;
    }

    void AeonEngineComponentsShutdown()
    {
        AeonGames::UnregisterComponentConstructor ( AeonGames::ModelComponent::GetClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::Camera::GetClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::OverTheShoulderCamera::GetClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::CharacterController::GetClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::FreeCamera::GetClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::PointLight::GetClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::SpotLight::GetClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::DirectionalLight::GetClassId() );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "AeonEngine Components",
        "Default Component Implementations",
        AeonEngineComponentsStartUp,
        AeonEngineComponentsShutdown
    };
}

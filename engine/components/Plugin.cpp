/*
Copyright (C) 2018,2019 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Plugin.h"
#include "aeongames/Component.h"
#include "aeongames/StringId.h"
#include "ModelComponent.h"
#include "Camera.h"
#include "PointLight.h"
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
        AeonGames::RegisterComponentConstructor ( AeonGames::PointLight::GetClassId(), []()
        {
            return std::make_unique<AeonGames::PointLight>();
        } );
        return true;
    }

    void AeonEngineComponentsShutdown()
    {
        AeonGames::UnregisterComponentConstructor ( AeonGames::ModelComponent::GetClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::Camera::GetClassId() );
        AeonGames::UnregisterComponentConstructor ( AeonGames::PointLight::GetClassId() );
    }

    PLUGIN PluginModuleInterface PMI =
    {
        "AeonEngine Components",
        "Default Component Implementations",
        AeonEngineComponentsStartUp,
        AeonEngineComponentsShutdown
    };
}

/*
Copyright (C) 2016,2018 Rodrigo Jose Hernandez Cordoba

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
#include <iostream>
#include <utility>
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/stubs/common.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Memory.h"
#include "aeongames/AeonEngine.h"
#include "aeongames/Renderer.h"
#include "aeongames/Scene.h"
#include "aeongames/GameWindow.h"

namespace AeonGames
{
    AeonEngine::AeonEngine() = default;
    AeonEngine::~AeonEngine() = default;

    void AeonEngine::Step ( double aDeltaTime )
    {
        if ( mScene )
        {
            mScene->Update ( aDeltaTime );
        }
    }

    int AeonEngine::Run()
    {
        /**@todo Implement basic game loop.*/
        return 0;
    }

    void AeonEngine::SetScene ( const std::shared_ptr<Scene>& aScene )
    {
        mScene = aScene;
    }

    const std::shared_ptr<Scene>& AeonEngine::GetScene() const
    {
        return mScene;
    }
}

/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
    struct AeonEngine::Impl
    {
        Impl()
        {
        }
        ~Impl()
            = default;
        std::shared_ptr<Scene> mScene = nullptr;
    };

    AeonEngine::AeonEngine() :
        pImpl ( std::make_unique<Impl>() )
    {
    }

    AeonEngine::~AeonEngine() = default;

    void AeonEngine::Step ( double aDeltaTime )
    {
        if ( pImpl->mScene )
        {
            pImpl->mScene->Update ( aDeltaTime );
        }
    }

    int AeonEngine::Run()
    {
        return 0;
    }

    void AeonEngine::SetScene ( std::shared_ptr<Scene> aScene )
    {
        pImpl->mScene = aScene;
    }

    std::shared_ptr<Scene> AeonEngine::GetScene() const
    {
        return pImpl->mScene;
    }

}

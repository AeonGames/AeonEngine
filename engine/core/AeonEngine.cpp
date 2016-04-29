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
#include "aeongames/AeonEngine.h"
#include "Vulkan.h"
#include <memory>

#if 0
// This doesn't work on MinGW
//__cplusplus < 201402L && __cplusplus >= 201103L
// Taken from EMC++ Item 21
namespace std
{
    template<typename T, typename... Ts>
    std::unique_ptr<T> make_unique ( Ts&&... params )
    {
        return std::unique_ptr<T> ( new T ( std::forward<Ts> ( params )... ) );
    }
}
#endif

namespace AeonGames
{
    struct AeonEngine::Impl
    {
        Vulkan mVulkan;
    };

    AeonEngine::AeonEngine() :
        pImpl ( std::make_unique<Impl>() )
    {
    }

    AeonEngine::~AeonEngine() = default;

    AeonEngine::AeonEngine ( AeonEngine && aRhs ) noexcept = default;

    AeonEngine & AeonEngine::operator= ( AeonEngine && aRhs ) noexcept = default;

    AeonEngine::AeonEngine ( const AeonEngine & aRhs )
        : pImpl ( nullptr )
    {
        if ( aRhs.pImpl )
        {
            pImpl = std::make_unique<Impl> ( *aRhs.pImpl );
        }
    }

    AeonEngine & AeonEngine::operator= ( const AeonEngine & aRhs )
    {
        if ( !aRhs.pImpl )
        {
            pImpl.reset();
        }
        else if ( !pImpl )
        {
            pImpl = std::make_unique<Impl> ( *aRhs.pImpl );
        }
        else
        {
            *pImpl = *aRhs.pImpl;
        }
        return *this;
    }
}

/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_GRAPHICALUSERINTERFACE_H
#define AEONGAMES_GRAPHICALUSERINTERFACE_H
#include <cassert>
#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include "aeongames/Platform.h"
#include "aeongames/StringId.h"

namespace AeonGames
{
    class GraphicalUserInterface
    {
    public:
        virtual void ResizeViewport ( uint32_t aWidth, uint32_t aHeight ) = 0;
        DLL virtual ~GraphicalUserInterface() = 0;
    };
    /**@name Factory Functions */
    /*@{*/
    /** Registers a GraphicalUserInterface loader for a specific identifier.*/
    DLL bool RegisterGraphicalUserInterfaceConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<GraphicalUserInterface>() >& aConstructor );
    /** Unregisters a GraphicalUserInterface loader for a specific identifier.*/
    DLL bool UnregisterGraphicalUserInterfaceConstructor ( const StringId& aIdentifier );
    /** Enumerates GraphicalUserInterface loader identifiers via an enumerator functor.*/
    DLL void EnumerateGraphicalUserInterfaceConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /*@}*/
}
#endif

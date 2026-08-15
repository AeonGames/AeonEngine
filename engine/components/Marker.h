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
#ifndef AEONGAMES_MARKER_H
#define AEONGAMES_MARKER_H
#include <string>
#include "aeongames/Component.hpp"

namespace AeonGames
{
    class Node;
    /** @brief Names a point in a scene: a spawn point, an attachment socket or
     *  any other location gameplay code looks up by node name. The marker is
     *  inert; all it carries is the Type used to tell one kind from another. */
    class Marker final : public Component
    {
    public:
        Marker();
        ~Marker() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const StringId* GetPropertyInfoArray () const final;
        Property GetProperty ( const StringId& aId ) const final;
        void SetProperty ( uint32_t, const Property& aProperty ) final;
        void Update ( Node& aNode, double aDelta ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        static const StringId& GetClassId();
    private:
        std::string mType{};
    };
}
#endif

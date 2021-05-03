/*
Copyright (C) 2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Component.h"

namespace AeonGames
{
    class Node;
    class Window;
    class PointLight final : public Component
    {
    public:
        PointLight();
        /** @name Overrides */
        ///@{
        ~PointLight() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const StringId* GetPropertyInfoArray () const final;
        Property GetProperty ( const StringId& aId ) const final;
        void SetProperty ( uint32_t, const Property& aProperty ) final;
        void Update ( Node& aNode, double aDelta, Window* aWindow = nullptr ) final;
        void Render ( const Node& aNode, const Window& aWindow ) const final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}
        static const StringId& GetClassId();
    private:
    };
}
#endif

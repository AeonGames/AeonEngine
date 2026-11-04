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
#ifndef AEONGAMES_TACTICALGRID_HPP
#define AEONGAMES_TACTICALGRID_HPP
#include "aeongames/ComponentProperties.hpp"

namespace AeonGames
{
    /** @brief Tactical grid component for managing grid-based tactical gameplay. */
    class TacticalGrid final : public ComponentProperties<TacticalGrid>
    {
    public:
        /** @brief Default constructor. */
        TacticalGrid();
        /** @name Overrides */
        ///@{
        ~TacticalGrid() final;
        const StringId& GetClassId() const final;
        void Update ( Node& aNode, double aDelta ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}
        /** @brief Returns the class identifier for the TacticalGrid component. */
        static const StringId& ClassId();
    private:
        friend class ComponentProperties<TacticalGrid>;
        inline static constexpr std::array<ComponentPropertyDescriptor<TacticalGrid>, 0> kPropertyDescriptors{};
    };
}

#endif // AEONGAMES_TACTICALGRID_HPP

/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MODELCONTROLLER_H
#define AEONGAMES_MODELCONTROLLER_H
#include <vector>
#include <tuple>
#include <array>
#include "aeongames/Component.h"

namespace AeonGames
{
    class Model;
    class ModelController : public Component
    {
    public:
        /** @name Component Overrides */
        ///@{
        const char* GetTypeName() const final;
        uint32_t GetTypeId() const final;
        std::vector<uint32_t> GetDependencies() const final;
        void Update ( Node& aNode, double aDelta ) final;
        void Render ( const Node& aNode, const Window& aWindow ) const final;
        /**@copydoc Component::EnumerateProperties */
        bool EnumerateProperties ( size_t* aPropertyCount, PropertyRecord* aRecords ) const final;
        /**@copydoc Component::SetProperty */
        void SetProperty ( const char* aName, const TypedPointer& aValue ) final;
        /**@copydoc Component::GetProperty */
        const TypedPointer GetProperty ( const char* aName ) const final;
        /**@copydoc Component::SetProperty */
        void SetProperty ( size_t aIndex, const TypedPointer& aValue ) final;
        /**@copydoc Component::GetProperty */
        const TypedPointer GetProperty ( size_t aIndex ) const final;
        ~ModelController() final;
        ///@}
    private:
        using ModelProperty = Property<sizeof ( std::string ) >;
        std::shared_ptr<Model> mModel{};
        std::array<ModelProperty, 3> mProperties
        {
            {
                {
                    "Model",
                    std::string{}
                },
                {
                    "Active Animation",
                    size_t{}
                },
                {
                    "Animation Delta",
                    double{}
                }
            }
        };
    };
}
#endif

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
#ifndef AEONGAMES_MODELDATA_H
#define AEONGAMES_MODELDATA_H
#include "aeongames/NodeData.h"
#include "aeongames/ResourceId.h"
#include "aeongames/UniformBuffer.h"

namespace AeonGames
{
    class ModelData : public NodeData
    {
    public:
        /** @name Overrides */
        ///@{
        ~ModelData() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const PropertyInfo* GetPropertyInfoArray () const final;
        const UntypedRef GetProperty ( const StringId& aId ) const final;
        void SetProperty ( const StringId& aId, const UntypedRef aRef ) final;
        ///@}

        /** @name Properties */
        ///@{
        void SetModel ( const ResourceId& aModel ) noexcept;
        const ResourceId& GetModel() const noexcept;
        void SetActiveAnimation ( size_t aActiveAnimation ) noexcept;
        const size_t& GetActiveAnimation() const noexcept;
        void SetAnimationDelta ( double aAnimationDelta ) noexcept;
        const double& GetAnimationDelta() const noexcept;
        ///@}
    private:
        // Properties
        ResourceId mModel{};
        size_t mActiveAnimation{};
        double mAnimationDelta{};
        // Private Data
        std::unique_ptr<UniformBuffer> mSkeletonBuffer{};
    };
}
#endif
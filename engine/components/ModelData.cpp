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

#include <array>
#include "ModelData.h"
#include "aeongames/PropertyInfo.h"

namespace AeonGames
{
    static const StringId ModelStringId{"ModelData"};

    ModelData::~ModelData() = default;

    const StringId& ModelData::GetId() const
    {
        return ModelStringId;
    }

    static const std::array<const PropertyInfo, 1> ModelDataPropertyInfo
    {
        {{"Model", typeid ( ResourceId ) }}
    };

    size_t ModelData::GetPropertyCount () const
    {
        return ModelDataPropertyInfo.size();
    }

    const PropertyInfo* ModelData::GetPropertyInfoArray () const
    {
        return ModelDataPropertyInfo.data();
    }


    const UntypedRef ModelData::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case "Model"_crc32:
            return GetModel();
        case "Active Animation"_crc32:
            return GetActiveAnimation();
        case "Animation Delta"_crc32:
            return GetAnimationDelta();
        }
        return UntypedRef{nullptr};
    }

    void ModelData::SetProperty ( const StringId& aId, const UntypedRef aRef )
    {
        switch ( aId )
        {
        case "Model"_crc32:
            SetModel ( aRef.Get<ResourceId>() );
            break;
        case "Active Animation"_crc32:
            SetActiveAnimation ( aRef.Get<size_t>() );
            break;
        case "Animation Delta"_crc32:
            SetAnimationDelta ( aRef.Get<double>() );
            break;
        }
    }

    void ModelData::SetModel ( const ResourceId& aModel ) noexcept
    {
        mModel = aModel;
    }

    const ResourceId& ModelData::GetModel() const noexcept
    {
        return mModel;
    }

    void ModelData::SetActiveAnimation ( size_t aActiveAnimation ) noexcept
    {
        mActiveAnimation = aActiveAnimation;
    }

    const size_t& ModelData::GetActiveAnimation() const noexcept
    {
        return mActiveAnimation;
    }

    void ModelData::SetAnimationDelta ( double aAnimationDelta ) noexcept
    {
        mAnimationDelta = aAnimationDelta;
    }

    const double& ModelData::GetAnimationDelta() const noexcept
    {
        return mAnimationDelta;
    }
}

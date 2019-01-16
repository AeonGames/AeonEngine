/*
Copyright (C) 2018,2019 Rodrigo Jose Hernandez Cordoba

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

namespace AeonGames
{
    static const StringId ModelStringId{"ModelData"};

    ModelData::~ModelData() = default;

    const StringId& ModelData::GetId() const
    {
        return ModelStringId;
    }

    static const std::array<const StringId, 3> ModelDataPropertyIds
    {
        {
            {"Model"},
            {"Active Animation"},
            {"Animation Delta"},
        }
    };

    size_t ModelData::GetPropertyCount () const
    {
        return ModelDataPropertyIds.size();
    }

    const StringId* ModelData::GetPropertyInfoArray () const
    {
        return ModelDataPropertyIds.data();
    }


    Property ModelData::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case "Model"_crc32:
            return GetModel().GetPathString();
        case "Active Animation"_crc32:
            return GetActiveAnimation();
        case "Animation Delta"_crc32:
            return GetAnimationDelta();
        }
        return Property{};
    }

    void ModelData::SetProperty ( const StringId& aId, const Property& aProperty )
    {
        switch ( aId )
        {
        case "Model"_crc32:
            SetModel ( {"Model"_crc32, std::get<std::string> ( aProperty ) } );
            break;
        case "Active Animation"_crc32:
            SetActiveAnimation ( std::get<size_t> ( aProperty ) );
            break;
        case "Animation Delta"_crc32:
            SetAnimationDelta ( std::get<double> ( aProperty ) );
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

/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include <vector>
#include "aeongames/Model.h"
#include "aeongames/ResourceCache.h"
#include "VulkanRenderer.h"
#include "VulkanModel.h"
#include "VulkanProgram.h"
#include "VulkanMesh.h"

namespace AeonGames
{
    VulkanModel::VulkanModel ( const std::shared_ptr<Model> aModel, const VulkanRenderer& aVulkanRenderer ) :
        mModel ( aModel ), mVulkanRenderer ( aVulkanRenderer )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    VulkanModel::~VulkanModel()
    {
        Finalize();
    }

    const std::shared_ptr<VulkanProgram> VulkanModel::GetProgram() const
    {
        return mProgram;
    }

    const std::shared_ptr<VulkanMesh> VulkanModel::GetMesh() const
    {
        return mMesh;
    }

    void VulkanModel::Initialize()
    {
        mProgram = Get<VulkanProgram> ( mModel->GetProgram() );
        mMesh = Get<VulkanMesh> ( mModel->GetMesh(), mVulkanRenderer );
    }

    void VulkanModel::Finalize()
    {
    }
}

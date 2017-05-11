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
#include "VulkanWindow.h"
#include "VulkanModel.h"
#include "VulkanPipeline.h"
#include "VulkanMesh.h"

namespace AeonGames
{
    VulkanModel::VulkanModel ( const std::shared_ptr<Model> aModel, const VulkanRenderer* aVulkanRenderer ) :
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

    void VulkanModel::Render ( const VulkanWindow& aWindow ) const
    {
        mProgram->Use ( aWindow );
        mMesh->Render();
    }

    void VulkanModel::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        mProgram = Get<VulkanPipeline> ( mModel->GetProgram(), mVulkanRenderer );
        mMesh = Get<VulkanMesh> ( mModel->GetMesh(), mVulkanRenderer );
    }

    void VulkanModel::Finalize()
    {
    }
}

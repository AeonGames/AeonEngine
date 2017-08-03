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
#ifndef AEONGAMES_VULKANMODEL_H
#define AEONGAMES_VULKANMODEL_H
#include "aeongames/RenderModel.h"

namespace AeonGames
{
    class Model;
    class VulkanRenderer;
    class VulkanPipeline;
    class VulkanMaterial;
    class VulkanMesh;
    class VulkanWindow;
    class VulkanModel : public RenderModel
    {
    public:
        VulkanModel ( const std::shared_ptr<Model> aModel, const std::shared_ptr<const VulkanRenderer> aVulkanRenderer );
        virtual ~VulkanModel();
        void Render ( const VulkanWindow& aWindow ) const;
        const std::shared_ptr<Model>& GetModel() const final;
    private:
        const std::shared_ptr<Model> mModel;
        std::shared_ptr<const VulkanRenderer> mVulkanRenderer;
        std::vector<std::tuple<
        std::shared_ptr<VulkanPipeline>,
            std::shared_ptr<VulkanMaterial>,
            std::shared_ptr<VulkanMesh>>> mMeshes;
        void Initialize();
        void Finalize();
    };
}
#endif

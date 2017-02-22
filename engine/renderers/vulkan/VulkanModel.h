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

namespace AeonGames
{
    class Model;
    class VulkanProgram;
    class VulkanMesh;
    class VulkanModel
    {
    public:
        VulkanModel ( const std::shared_ptr<Model> aModel );
        ~VulkanModel();
        const std::shared_ptr<VulkanProgram> GetProgram() const;
        const std::shared_ptr<VulkanMesh> GetMesh() const;
    private:
        void Initialize();
        void Finalize();
        const std::shared_ptr<Model> mModel;
        std::shared_ptr<VulkanProgram> mProgram;
        std::shared_ptr<VulkanMesh> mMesh;
    };
}
#endif

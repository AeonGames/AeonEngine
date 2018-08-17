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
#ifndef AEONGAMES_MESHCOMPONENT_H
#define AEONGAMES_MESHCOMPONENT_H
#include "aeongames/Component.h"
namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class MeshComponent : public Component
    {
    public:
        ~MeshComponent() final;
        uint32_t GetTypeId() const final;
        std::vector<uint32_t> GetDependencies() const final;
        uint32_t GetPipeline() const;
        void SetPipeline ( uint32_t aPipelineId );
        uint32_t GetMesh() const;
        void SetMesh ( uint32_t aMeshId );
        void Update ( Node& aNode, double aDelta ) final;
        void Render ( const Node& aNode, const Window& aWindow ) const final;
    private:
        uint32_t mPipelineId{};
        std::shared_ptr<Pipeline> mPipeline{nullptr};
        uint32_t mMeshId{};
        std::shared_ptr<Mesh> mMesh{nullptr};
    };
}
#endif

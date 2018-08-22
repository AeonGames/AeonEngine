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
#ifndef AEONGAMES_MODELCOMPONENT_H
#define AEONGAMES_MODELCOMPONENT_H
#include <vector>
#include <tuple>
#include "aeongames/Component.h"
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "model.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class ModelComponent : public Component
    {
    public:
        ~ModelComponent() final;
        uint32_t GetTypeId() const final;
        std::vector<uint32_t> GetDependencies() const final;
        void Update ( Node& aNode, double aDelta ) final;
        void Render ( const Node& aNode, const Window& aWindow ) const final;
        const google::protobuf::Message* GetProperties() const final;
    private:
        ModelBuffer mProperties{};
        std::vector<std::tuple<
        std::shared_ptr<Mesh>,
            std::shared_ptr<Pipeline>,
            std::shared_ptr<Material>>
            > mMeshes{};
    };
}
#endif

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
#include "aeongames/Component.h"
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "modelcontroller.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    class Model;
    class ModelController : public Component
    {
    public:
        /** @name Component Overrides */
        ///@{
        ~ModelController() final;
        const std::string& GetTypeName() const final;
        uint32_t GetTypeId() const final;
        std::vector<uint32_t> GetDependencies() const final;
        void Update ( Node& aNode, double aDelta ) final;
        void Render ( const Node& aNode, const Window& aWindow ) const final;
        void CommitPropertyChanges() final;
        const google::protobuf::Message* GetProperties() const final;
        google::protobuf::Message* GetProperties() final;
        ///@}
    private:
        std::shared_ptr<Model> mModel{};
        ModelControllerBuffer mProperties{};
    };
}
#endif

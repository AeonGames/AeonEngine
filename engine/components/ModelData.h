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

namespace AeonGames
{
    class ModelData : public NodeData
    {
    public:
        ~ModelData() final;
        const uint32_t GetTypeId() const final;
        const char* GetTypeName() const final;
        static const NodeData::TypeInfo mTypeInfo;
    private:
        ResourceId mModel{};
        size_t mActiveAnimation{};
        double mAnimationDelta{};
    };
}
#endif
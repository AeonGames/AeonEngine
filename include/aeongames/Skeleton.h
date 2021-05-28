/*
Copyright (C) 2017-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_SKELETON_H
#define AEONGAMES_SKELETON_H
#include <cstdint>
#include <vector>
#include <string>
#include "aeongames/Transform.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "skeleton.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Resource.h"

namespace AeonGames
{
    class SkeletonMsg;
    class Skeleton : public Resource<SkeletonMsg, "AEONSKL"_mgk>
    {
    public:
        class Joint
        {
        public:
            Joint ( Joint* aParent, const Transform& aTransform, const Transform& aInvertedTransform, const std::string& aName );
            DLL const Transform& GetTransform() const;
            DLL const Transform& GetInvertedTransform() const;
            DLL const Joint* GetParent() const;
        private:
            Joint* mParent = nullptr;
            Transform mTransform;
            Transform mInvertedTransform;
            std::string mName;
        };
        DLL Skeleton();
        DLL ~Skeleton();
        DLL void Unload () final;
        DLL const std::vector<Joint>& GetJoints() const;
    private:
        void Load ( const SkeletonMsg& aSkeletonMsg );
        std::vector<Joint> mJoints{};
    };
}
#endif

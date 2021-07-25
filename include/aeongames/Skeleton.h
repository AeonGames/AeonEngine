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
#include "aeongames/Resource.h"

namespace AeonGames
{
    class SkeletonMsg;
    class Animation;
    class Skeleton : public Resource
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
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        DLL void LoadFromPBMsg ( const SkeletonMsg& aSkeletonMsg );
        DLL void Unload () final;
        DLL const std::vector<Joint>& GetJoints() const;
    private:
        std::vector<Joint> mJoints{};
    };
}
#endif

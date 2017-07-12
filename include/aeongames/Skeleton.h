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
#ifndef AEONGAMES_SKELETON_H
#define AEONGAMES_SKELETON_H
#include <cstdint>
#include <vector>
#include "aeongames/Transform.h"
namespace AeonGames
{
    class Skeleton
    {
    public:
        class Joint
        {
        public:
            Joint ( Joint* aParent, const Transform& aTransform, const Transform& aInvertedTransform, const std::string& aName );
            DLL const Transform& GetTransform() const;
        private:
            Joint* mParent = nullptr;
            Transform mTransform;
            Transform mInvertedTransform;
            std::string mName;
        };
        Skeleton ( std::string  aFilename );
        ~Skeleton();
        DLL const std::vector<Joint>& GetJoints() const;
    private:
        void Initialize();
        void Finalize();
        std::string mFilename;
        std::vector<Joint> mJoints;
    };
}
#endif

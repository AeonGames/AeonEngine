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
#ifndef AEONGAMES_MODELINSTANCE_H
#define AEONGAMES_MODELINSTANCE_H
#include <vector>
#include "aeongames/Memory.h"
#include "aeongames/Platform.h"

namespace AeonGames
{
    class Model;
    class Matrix4x4;
    class ModelInstance
    {
    public:
        DLL ModelInstance ( const std::shared_ptr<const Model> aModel );
        DLL ~ModelInstance();
        DLL void EnableAssembly ( size_t aAssemblyIndex, bool aEnabled = true );
        DLL void EnableAllAssemblies ( bool aEnabled = true );
        DLL size_t GetAnimationIndex ( size_t aAnimationIndex ) const;
        DLL void SetAnimationIndex ( size_t aAnimationIndex );
        DLL float GetAnimationTime() const;
        DLL void  SetAnimationTime ( float aTime = 0.0f );
        DLL void StepAnimation ( float aDelta );
        DLL const std::vector<Matrix4x4>& GetSkeletonAnimation() const;
    private:
        void Initialize();
        void Finalize();
        void UpdateSkeletonAnimation();
        const std::shared_ptr<const Model> mModel;
        std::vector<bool> mEnabledAssemblies;
        size_t mAnimationIndex;
        float mAnimationTime;
        std::vector<Matrix4x4> mSkeletonAnimation;
    };
}
#endif

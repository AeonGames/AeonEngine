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
#ifndef AEONGAMES_ANIMATION_H
#define AEONGAMES_ANIMATION_H
#include <cstdint>
#include <vector>
#include "aeongames/Platform.h"
#include "aeongames/Transform.h"
#include "aeongames/Resource.h"

namespace AeonGames
{
    class AnimationMsg;
    class Animation : public Resource
    {
    public:
        DLL Animation();
        DLL ~Animation();
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        DLL void Unload () final;
        DLL uint32_t GetFrameRate() const;
        DLL double GetDuration() const;
        DLL double GetSample ( double aTime ) const;
        DLL double AddTimeToSample ( double aSample, double aTime ) const;
        DLL const Transform GetTransform ( size_t aBoneIndex, double aSample ) const;
        DLL void LoadFromPBMsg ( const AnimationMsg& aAnimationMsg );
    private:
        std::string mFilename;
        uint32_t mVersion;
        uint32_t mFrameRate;
        double mDuration;
        std::vector<std::vector<Transform>> mFrames;
    };
}
#endif

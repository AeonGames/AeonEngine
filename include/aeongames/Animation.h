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
#include "ProtoBufHelpers.h"
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "vector3.pb.h"
#include "quaternion.pb.h"
#include "animation.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Resource.h"

namespace AeonGames
{
    class AnimationMsg;
    class Animation : public Resource<AnimationMsg, "AEONANM"_mgk>
    {
    public:
        DLL Animation();
        DLL Animation ( uint32_t aId );
        DLL Animation ( const std::string& aFilename );
        DLL Animation ( const void* aBuffer, size_t aBufferSize );
        DLL ~Animation();
        DLL void Unload () final;
        DLL uint32_t GetFrameRate() const;
        DLL double GetDuration() const;
        DLL double GetSample ( double aTime ) const;
        DLL double AddTimeToSample ( double aSample, double aTime ) const;
        DLL const Transform GetTransform ( size_t aBoneIndex, double aSample ) const;
    private:
        void Load ( const AnimationMsg& aAnimationMsg ) final;
        std::string mFilename;
        uint32_t mVersion;
        uint32_t mFrameRate;
        double mDuration;
        std::vector<std::vector<Transform>> mFrames;
    };
}
#endif

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

#include <exception>
#include <vector>
#include <cassert>
#include <cstring>
#include <cmath>
#include <mutex>
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"
#include "aeongames/Utilities.h"
#include "aeongames/Animation.h"
#include "aeongames/ResourceCache.h"

namespace AeonGames
{

    Animation::Animation()
        = default;
    void Animation::Load ( const AnimationMsg& aAnimationMsg )
    {
        mVersion = aAnimationMsg.version();
        mFrameRate = aAnimationMsg.framerate();
        mDuration = aAnimationMsg.duration();
        mFrames.reserve ( aAnimationMsg.frame_size() );
        for ( auto& frame : aAnimationMsg.frame() )
        {
            mFrames.emplace_back();
            mFrames.back().reserve ( frame.bone_size() );
            for ( auto& joint : frame.bone() )
            {
                mFrames.back().emplace_back (
                    Transform
                {
                    Vector3
                    {
                        joint.scale().x(),
                        joint.scale().y(),
                        joint.scale().z()
                    },
                    Quaternion
                    {
                        joint.rotation().w(),
                        joint.rotation().x(),
                        joint.rotation().y(),
                        joint.rotation().z()
                    },
                    Vector3
                    {
                        joint.translation().x(),
                        joint.translation().y(),
                        joint.translation().z()
                    }                   }
                );
            }
        }
    }

    void Animation::Unload()
    {
        mVersion = 0;
        mFrameRate = 0;
        mDuration = 0;
        mFrames.clear();
    }

    Animation::~Animation() = default;

    uint32_t Animation::GetFrameRate() const
    {
        return mFrameRate;
    }

    double Animation::GetDuration() const
    {
        return mDuration;
    }

    double Animation::GetSample ( double aTime ) const
    {
        /**
         * The sample integral part represents the initial frame,
         * the fractional part is the interpolation between the initial frame
         * and the next.
        */
        return fmod ( mFrameRate * fmod ( aTime, mDuration ), mFrames.size() );
    }

    double Animation::AddTimeToSample ( double aSample, double aTime ) const
    {
        return fmod ( aSample + ( mFrameRate * fmod ( aTime, mDuration ) ), mFrames.size() );
    }

    const Transform Animation::GetTransform ( size_t aBoneIndex, double aSample ) const
    {
        double frame;
        double interpolation = modf ( aSample, &frame );
        auto frame1 = static_cast<size_t> ( frame );
        size_t frame2 = ( ( frame1 + 1 ) % mFrames.size() );
        size_t frame0 = frame1 == 0 ? mFrames.size() - 1 : ( ( frame1 - 1 ) % mFrames.size() );
        size_t frame3 = ( ( frame1 + 2 ) % mFrames.size() );
        /// modf should guarantee interpolation to be in the range [0.0,1.0)
        assert ( ( interpolation >= 0.0 ) && ( interpolation < 1.0 ) && "Interpolation out of range [0.0,1.0)." );
        return Interpolate ( mFrames[frame0][aBoneIndex], mFrames[frame1][aBoneIndex], mFrames[frame2][aBoneIndex], mFrames[frame3][aBoneIndex], interpolation );
    }
}

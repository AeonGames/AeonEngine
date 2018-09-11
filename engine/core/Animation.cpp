/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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

#include <fstream>
#include <sstream>
#include <exception>
#include <vector>
#include <cassert>
#include <cstring>
#include <cmath>
#include "aeongames/AeonEngine.h"
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "animation.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/CRC.h"
#include "aeongames/Utilities.h"
#include "aeongames/Animation.h"
#include "aeongames/ResourceCache.h"

namespace AeonGames
{

    Animation::Animation()
        = default;

    Animation::Animation ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        try
        {
            Load ( buffer.data(), buffer.size() );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Animation::Animation ( const std::string&  aFilename )
    {
        try
        {
            Load ( aFilename );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Animation::Animation ( const void * aBuffer, size_t aBufferSize )
    {
        if ( !aBuffer && !aBufferSize )
        {
            throw std::runtime_error ( "Cannot initialize Animation object with null data." );
            return;
        }
        try
        {
            Load ( aBuffer, aBufferSize );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    void Animation::Load ( const std::string& aFilename )
    {
        static std::mutex m;
        static AnimationBuffer skeleton_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<AnimationBuffer> ( skeleton_buffer, aFilename, "AEONANM" );
        Load ( skeleton_buffer );
        skeleton_buffer.Clear();
    }

    void Animation::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static std::mutex m;
        static AnimationBuffer skeleton_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<AnimationBuffer> ( skeleton_buffer, aBuffer, aBufferSize, "AEONANM" );
        Load ( skeleton_buffer );
        skeleton_buffer.Clear();
    }

    void Animation::Load ( const AnimationBuffer& aAnimationBuffer )
    {
        mVersion = aAnimationBuffer.version();
        mFrameRate = aAnimationBuffer.framerate();
        mDuration = aAnimationBuffer.duration();
        mFrames.reserve ( aAnimationBuffer.frame_size() );
        for ( auto& frame : aAnimationBuffer.frame() )
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

    const Transform Animation::GetTransform ( size_t aBoneIndex, double aTime ) const
    {
        double sample = fmod ( mFrameRate * fmod ( aTime, mDuration ), mFrames.size() );
        double frame;
        double interpolation = modf ( sample, &frame );
        auto frame1 = static_cast<size_t> ( frame );
        size_t frame2 = ( ( frame1 + 1 ) % mFrames.size() );
        size_t frame0 = frame1 == 0 ? mFrames.size() - 1 : ( ( frame1 - 1 ) % mFrames.size() );
        size_t frame3 = ( ( frame1 + 2 ) % mFrames.size() );
        if ( interpolation <= 0.0 )
        {
            return mFrames[frame1][aBoneIndex];
        }
        else if ( interpolation >= 1.0 )
        {
            return mFrames[frame2][aBoneIndex];
        }
        return Interpolate ( mFrames[frame0][aBoneIndex], mFrames[frame1][aBoneIndex], mFrames[frame2][aBoneIndex], mFrames[frame3][aBoneIndex], interpolation );
    }

    const std::shared_ptr<Animation> Animation::GetAnimation ( uint32_t aId )
    {
        return Get<Animation> ( aId, aId );
    }

    const std::shared_ptr<Animation> Animation::GetAnimation ( const std::string& aPath )
    {
        uint32_t id = crc32i ( aPath.c_str(), aPath.size() );
        return Animation::GetAnimation ( id );
    }
}

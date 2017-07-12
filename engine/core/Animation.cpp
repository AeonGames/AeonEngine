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

#include <fstream>
#include <sstream>
#include <exception>
#include <vector>
#include <cassert>
#include <cstring>
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "Animation.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/Utilities.h"
#include "aeongames/Animation.h"

namespace AeonGames
{
    Animation::Animation ( std::string  aFilename )
        :
        mFilename ( std::move ( aFilename ) )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    Animation::~Animation()
    {
        Finalize();
    }

    void Animation::Initialize()
    {
        static AnimationBuffer animation_buffer;
        LoadProtoBufObject<AnimationBuffer> ( animation_buffer, mFilename, "AEONANM" );
        mFrameRate = animation_buffer.framerate();
        mDuration = animation_buffer.duration();
        mFrames.reserve ( animation_buffer.frame_size() );
        for ( auto& frame : animation_buffer.frame() )
        {
            mFrames.emplace_back();
            mFrames.back().reserve ( frame.bone_size() );
            for ( auto& joint : frame.bone() )
            {
                mFrames.back().emplace_back (
                    Transform (
                        joint.translation().x(),
                        joint.translation().y(),
                        joint.translation().z(),
                        joint.rotation().w(),
                        joint.rotation().x(),
                        joint.rotation().y(),
                        joint.rotation().z(),
                        joint.scale().x(),
                        joint.scale().y(),
                        joint.scale().z()
                    )
                );
            }
        }
        animation_buffer.Clear();
    }

    void Animation::Finalize()
    {
    }
}

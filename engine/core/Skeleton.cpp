/*
Copyright (C) 2017-2019 Rodrigo Jose Hernandez Cordoba

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
#include <mutex>
#include "aeongames/AeonEngine.h"
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "skeleton.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/CRC.h"
#include "aeongames/Utilities.h"
#include "aeongames/Skeleton.h"
#include "aeongames/ResourceCache.h"

namespace AeonGames
{
    Skeleton::Joint::Joint ( Joint * aParent, const Transform & aTransform, const Transform & aInvertedTransform, const std::string & aName ) :
        mParent ( aParent ), mTransform ( aTransform ), mInvertedTransform ( aInvertedTransform  ), mName (  aName )
    {
    }

    const Transform & Skeleton::Joint::GetTransform() const
    {
        return mTransform;
    }

    const Transform & Skeleton::Joint::GetInvertedTransform() const
    {
        return mInvertedTransform;
    }

    const Skeleton::Joint * Skeleton::Joint::GetParent() const
    {
        return mParent;
    }


    Skeleton::Skeleton()
        = default;

    Skeleton::Skeleton ( uint32_t aId )
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

    Skeleton::Skeleton ( const std::string&  aFilename )
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

    Skeleton::Skeleton ( const void * aBuffer, size_t aBufferSize )
    {
        if ( !aBuffer && !aBufferSize )
        {
            throw std::runtime_error ( "Cannot initialize Skeleton object with null data." );
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


    Skeleton::~Skeleton() = default;

    void Skeleton::Load ( const std::string& aFilename )
    {
        static std::mutex m;
        static SkeletonBuffer skeleton_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<SkeletonBuffer> ( skeleton_buffer, aFilename, "AEONSKL" );
        Load ( skeleton_buffer );
        skeleton_buffer.Clear();
    }

    void Skeleton::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static std::mutex m;
        static SkeletonBuffer skeleton_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<SkeletonBuffer> ( skeleton_buffer, aBuffer, aBufferSize, "AEONSKL" );
        Load ( skeleton_buffer );
        skeleton_buffer.Clear();
    }

    void Skeleton::Load ( const SkeletonBuffer& aSkeletonBuffer )
    {
        mJoints.reserve ( aSkeletonBuffer.joint_size() );
        for ( auto& joint : aSkeletonBuffer.joint() )
        {
            mJoints.emplace_back (
                ( joint.parentindex() < 0 ) ? nullptr : &mJoints[joint.parentindex()],
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
                }
            },
            Transform
            {
                Vector3
                {
                    joint.invertedscale().x(),
                    joint.invertedscale().y(),
                    joint.invertedscale().z()
                },
                Quaternion
                {
                    joint.invertedrotation().w(),
                    joint.invertedrotation().x(),
                    joint.invertedrotation().y(),
                    joint.invertedrotation().z()
                },
                Vector3
                {
                    joint.invertedtranslation().x(),
                    joint.invertedtranslation().y(),
                    joint.invertedtranslation().z()
                }
            },
            joint.name() );
        }
    }

    void Skeleton::Unload()
    {
        mJoints.clear();
    }

    const std::vector<Skeleton::Joint>& Skeleton::GetJoints() const
    {
        return mJoints;
    }
}

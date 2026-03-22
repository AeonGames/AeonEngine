/*
Copyright (C) 2017-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Transform.hpp"
#include "aeongames/Resource.hpp"

namespace AeonGames
{
    class SkeletonMsg;
    class Animation;
    /** Skeletal animation resource containing a hierarchy of joints. */
    class Skeleton : public Resource
    {
    public:
        /** A single joint within a skeleton hierarchy. */
        class Joint
        {
        public:
            /** Construct a joint.
                @param aParent Pointer to the parent joint, or nullptr for a root joint.
                @param aTransform The local bind-pose transform.
                @param aInvertedTransform The inverse bind-pose transform.
                @param aName Name of the joint. */
            Joint ( Joint* aParent, const Transform& aTransform, const Transform& aInvertedTransform, const std::string& aName );
            /** Get the local bind-pose transform.
                @return Const reference to the transform. */
            DLL const Transform& GetTransform() const;
            /** Get the inverse bind-pose transform.
                @return Const reference to the inverted transform. */
            DLL const Transform& GetInvertedTransform() const;
            /** Get the parent joint.
                @return Pointer to the parent joint, or nullptr for a root joint. */
            DLL const Joint* GetParent() const;
        private:
            Joint* mParent = nullptr;
            Transform mTransform;
            Transform mInvertedTransform;
            std::string mName;
        };
        /** Construct an empty skeleton. */
        DLL Skeleton();
        /** Destructor. */
        DLL ~Skeleton();
        /** Load the skeleton from a memory buffer.
            @param aBuffer Pointer to the data buffer.
            @param aBufferSize Size of the buffer in bytes. */
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        /** Load the skeleton from a protobuf message.
            @param aSkeletonMsg The skeleton message to read from. */
        DLL void LoadFromPBMsg ( const SkeletonMsg& aSkeletonMsg );
        /** Unload and release skeleton data. */
        DLL void Unload () final;
        /** Get the joint list.
            @return Const reference to the vector of joints. */
        DLL const std::vector<Joint>& GetJoints() const;
    private:
        std::vector<Joint> mJoints{};
    };
}
#endif

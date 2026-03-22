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
#ifndef AEONGAMES_ANIMATION_H
#define AEONGAMES_ANIMATION_H
#include <cstdint>
#include <vector>
#include "aeongames/Platform.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Resource.hpp"

namespace AeonGames
{
    class AnimationMsg;
    /** @brief Stores skeletal animation data including keyframes and bone transforms. */
    class Animation : public Resource
    {
    public:
        DLL Animation();
        DLL ~Animation();
        /** @brief Load animation data from a memory buffer.
         *  @param aBuffer Pointer to the buffer containing animation data.
         *  @param aBufferSize Size of the buffer in bytes. */
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        /** @brief Unload animation data and free resources. */
        DLL void Unload () final;
        /** @brief Get the frame rate of the animation.
         *  @return Frame rate in frames per second. */
        DLL uint32_t GetFrameRate() const;
        /** @brief Get the total duration of the animation.
         *  @return Duration in seconds. */
        DLL double GetDuration() const;
        /** @brief Get a normalized sample position for a given time.
         *  @param aTime Time in seconds.
         *  @return Normalized sample position. */
        DLL double GetSample ( double aTime ) const;
        /** @brief Advance a sample position by a time delta.
         *  @param aSample Current sample position.
         *  @param aTime Time delta in seconds.
         *  @return New sample position. */
        DLL double AddTimeToSample ( double aSample, double aTime ) const;
        /** @brief Get the transform for a specific bone at a given sample.
         *  @param aBoneIndex Index of the bone in the skeleton.
         *  @param aSample Sample position within the animation.
         *  @return Transform for the bone at the given sample. */
        DLL const Transform GetTransform ( size_t aBoneIndex, double aSample ) const;
        /** @brief Load animation data from a protobuf message.
         *  @param aAnimationMsg Protobuf message containing animation data. */
        DLL void LoadFromPBMsg ( const AnimationMsg& aAnimationMsg );
    private:
        std::string mFilename;
        uint32_t mVersion;
        uint32_t mFrameRate;
        double mDuration;
        std::vector<std::vector<Transform >> mFrames;
    };
}
#endif

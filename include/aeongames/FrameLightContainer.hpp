/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_FRAMELIGHTCONTAINER_HPP
#define AEONGAMES_FRAMELIGHTCONTAINER_HPP

#include <cstddef>
#include <span>
#include <vector>
#include "aeongames/GpuLight.hpp"

namespace AeonGames
{
    /** @brief Per-scene CPU-side container for lights submitted by
     *  light components during a frame.
     *
     *  Backed by a @c std::vector capped at @ref MAX_LIGHTS_PER_FRAME. The
     *  cap is large (thousands of lights) since Clustered Forward+ culls the
     *  set per cluster, so the backing store is heap-allocated rather than an
     *  inline array to keep whichever subsystem composes one (Scene today,
     *  future deferred-pass packers tomorrow) small. Capacity is reserved
     *  lazily and reused across frames, so steady-state frames allocate
     *  nothing.
     *
     *  @note Not thread-safe. Safe today because Scene::Update runs a
     *  sequential DFS traversal, so all Add() calls happen on one thread
     *  and Get()/Reset() are only used outside the update phase. */
    class FrameLightContainer
    {
    public:
        /** @brief Append a light record. Drops the call silently once
         *  @ref MAX_LIGHTS_PER_FRAME entries are queued. */
        void Add ( const GpuLight& aLight )
        {
            if ( mLights.size() >= MAX_LIGHTS_PER_FRAME )
            {
                return;
            }
            mLights.emplace_back ( aLight );
        }
        /** @brief Read-only view of the lights queued this frame. */
        std::span<const GpuLight> Get() const
        {
            return { mLights.data(), mLights.size() };
        }
        /** @brief Drop all queued lights, keeping the backing capacity so
         *  subsequent frames reuse it without reallocating. */
        void Reset()
        {
            mLights.clear();
        }
    private:
        std::vector<GpuLight> mLights{};
    };
}
#endif

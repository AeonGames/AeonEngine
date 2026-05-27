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

#include <array>
#include <cstddef>
#include <span>
#include "aeongames/GpuLight.hpp"

namespace AeonGames
{
    /** @brief Per-scene CPU-side container for lights submitted by
     *  light components during a frame.
     *
     *  Backed by an inline @c std::array sized to @ref MAX_LIGHTS_PER_FRAME
     *  plus a running count, so no heap allocations ever happen and the
     *  payload lives in-place inside whichever subsystem composes one
     *  (Scene today, future deferred-pass packers tomorrow).
     *
     *  @note Not thread-safe. Safe today because Scene::Update runs a
     *  sequential DFS traversal, so all Add() calls happen on one thread
     *  and Get()/Reset() are only used outside the update phase. If node
     *  updates ever get parallelized, switch @c mCount to
     *  @c std::atomic<std::size_t> and have Add() reserve a slot with
     *  @c fetch_add(1, std::memory_order_relaxed) — writes to disjoint
     *  array elements are race-free, so no mutex is needed. */
    class FrameLightContainer
    {
    public:
        /** @brief Append a light record. Drops the call silently once
         *  @ref MAX_LIGHTS_PER_FRAME entries are queued. */
        void Add ( const GpuLight& aLight )
        {
            if ( mCount >= MAX_LIGHTS_PER_FRAME )
            {
                return;
            }
            mLights[mCount++] = aLight;
        }
        /** @brief Read-only view of the lights queued this frame. */
        std::span<const GpuLight> Get() const
        {
            return { mLights.data(), mCount };
        }
        /** @brief Drop all queued lights without touching the backing
         *  storage (the array stays in-place). */
        void Reset()
        {
            mCount = 0;
        }
    private:
        std::size_t mCount{};
        std::array<GpuLight, MAX_LIGHTS_PER_FRAME> mLights{};
    };
}
#endif


/*
Copyright (C) 2012,2019 Rodrigo Jose Hernandez Cordoba

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

#include "aeongames/Clock.h"

namespace AeonGames
{
    Clock::Clock() : mTime{}, mScale{1.0}, mPaused{}
    {
    }

    Clock::Clock ( double starttime ) :
        mTime {static_cast<uint64_t> ( starttime * 1e+9 ) },
        mScale { 1.0 },
        mPaused {}
    {
    }

    uint64_t Clock::GetTime() const
    {
        return mTime;
    }

    double Clock::GetDelta ( const Clock& clock )
    {
        return ( mTime - clock.mTime ) * 1e-9f;
    }

    double Clock::GetDelta ( const uint64_t& start_time )
    {
        return ( mTime - start_time ) * 1e-9f;
    }

    void Clock::Update ( double seconds )
    {
        if ( !mPaused )
        {
            mTime += static_cast<uint64_t> ( seconds * mScale * 1e+9f );
        }
    }

    void  Clock::Pause ( bool pause )
    {
        mPaused = pause;
    }

    bool  Clock::IsPaused()
    {
        return mPaused;
    }

    void  Clock::SetTimeScale ( double timescale )
    {
        mScale = timescale;
    }

    double Clock::GetTimeScale()
    {
        return mScale;
    }

    void  Clock::SingleStep()
    {
        if ( mPaused )
        {
            mTime += static_cast<uint64_t> ( ( 1.0 / 30.0 ) * mScale * 1e+9 );
        }
    }
}

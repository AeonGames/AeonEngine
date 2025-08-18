/*
Copyright (C) 2012,2019,2025 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_CLOCK_H
#define AEONGAMES_CLOCK_H
/*! \file
    \brief Header file for the Clock class
    \author Rodrigo Hernandez.
    &copy; 2012,2019 Rodrigo Hernandez
*/

#include <cstdint>
#include "aeongames/Platform.hpp"

namespace AeonGames
{
    class Clock
    {
    public:
        /// Construct a clock with local time 0
        DLL Clock();
        /*! Construct a clock with a provided local time.
            \param starttime [in] starting time for the clock.
        */
        DLL explicit Clock ( double starttime );
        /*! Get The current time for the clock in nanoseconds
            \return Absolute clock time in nanoseconds.*/
        DLL uint64_t GetTime() const;
        /*! Get The difference between this clock and another clock in seconds and fraction.
            \param clock [in] Clock to get the delta from.
            \return delta clock time in seconds.*/
        DLL double GetDelta ( const Clock& clock );
        /*! Get The difference between this clock and a previously recorded time in seconds and fraction.
            \param start_time [in] previously recorded time in nanoseconds.
            \return delta clock time in seconds.*/
        DLL double GetDelta ( const uint64_t& start_time );

        /*! Update the clock local time.
            \param seconds [in] Elapsed time since last call in seconds and fraction.*/
        DLL void Update ( double seconds );
        /*! Pause or resume clock depending on the parameter.
            \param pause [in] Boolean specifing if the clock is to be paused or resumed.
            \note Multiple calls passing the same value for the pause parameter will have no effect past the first one.
        */
        DLL void Pause ( bool pause = true );
        /*!  Query for the pause status of the clock.
            \param pause [in] Boolean specifing if the clock is to be paused or resumed.
            \note Multiple calls passing the same value for the pause parameter will have no effect past the first one.
        */
        DLL bool IsPaused();
        /*! Set the time scale.
            By default the time scale is set to no scale (1.0), this scale can be changed for a number higher than 1.0
            to speed up time or less than 1.0 to slow it down, scale can also be set to a negative number in which case
            time runs backwards or set to zero which frezes time.
            \param timescale [in] scale to set the clock to.
        */
        DLL void SetTimeScale ( double timescale );
        /*! Query time scale.
            \return clock time scale.
        */
        DLL double GetTimeScale();
        /*  Single step the clock when paused.
            This function only takes effect when the clock is paused, its uses are better suited for debbuging rather than gameplay.
        */
        DLL void SingleStep();
    private:
        /// Time in nanoseconds.
        uint64_t mTime;
        /// Clock scale.
        double    mScale;
        /// Is the clock paused.
        bool     mPaused;
    };
}
#endif

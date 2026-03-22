//
// Copyright (C) 2013,2026 LunarG, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
#ifndef WORKLIST_H_INCLUDED
#define WORKLIST_H_INCLUDED

#include <string>
#include <list>
#include <mutex>

namespace glslang
{

    /// @brief Represents a single work item in the compilation worklist.
    class TWorkItem
    {
    public:
        TWorkItem() { }
        /// @brief Construct from a file name.
        explicit TWorkItem ( const std::string& s ) :
            name ( s ) { }
        std::string name; ///< File name of the work item.
        std::string results; ///< Compilation results.
        std::string resultsIndex; ///< Index into the results.
    };

    /// @brief Thread-safe worklist for parallel shader compilation.
    class TWorklist
    {
    public:
        TWorklist() { }
        virtual ~TWorklist() { }

        /// @brief Add a work item to the worklist.
        void add ( TWorkItem* item )
        {
            std::lock_guard<std::mutex> hold ( mutex );
            worklist.push_back ( item );
        }

        /// @brief Remove and return the next work item. Returns false if empty.
        bool remove ( TWorkItem*& item )
        {
            std::lock_guard<std::mutex> hold ( mutex );
            if ( worklist.empty() )
            {
                return false;
            }
            item = worklist.front();
            worklist.pop_front();
            return true;
        }

        /// @brief Get the number of remaining work items.
        int size()
        {
            return ( int ) worklist.size();
        }

        /// @brief Check whether the worklist is empty.
        bool empty()
        {
            return worklist.empty();
        }

    protected:
        std::mutex mutex; ///< Mutex guarding the worklist.
        std::list<TWorkItem*> worklist; ///< Queue of pending work items.
    };

} // end namespace glslang

#endif // WORKLIST_H_INCLUDED

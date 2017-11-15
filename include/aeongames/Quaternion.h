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

#ifndef AEONGAMES_QUATERNION_H
#define AEONGAMES_QUATERNION_H
/*! \file
    \brief Header for the quaternion class.
    \author Rodrigo Hernandez.
    \copy 2017
*/

#include "aeongames/Platform.h"

namespace AeonGames
{
    /*! \brief Quaternion class. */
    class Quaternion
    {
    public:
        ///@brief Default constructor.
        DLL Quaternion();
        /// destructor.
        DLL ~Quaternion();
    protected:
        float mQuaternion[4] {};
    };
}
#endif

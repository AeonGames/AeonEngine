/*
Copyright (C) 2015-2018,2025 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_VECTOR4_H
#define AEONGAMES_VECTOR4_H
/*! \file
    \brief Header for the 4D vector class.
    \author Rodrigo Hernandez.
    \copy 2015-2017
*/

#include "aeongames/Platform.hpp"

namespace AeonGames
{
    /*! \brief 4D vector class. */
    class Vector4
    {
    public:
        ///@brief Default constructor.
        DLL Vector4();
        /*! @brief Constructor
        @param aVector a float pointer or array containing vector data.
        @note This is the same format the values are internally stored and what Vector3::GetVector3 returns.
        */
        DLL Vector4 ( const float* const aVector );
        DLL Vector4 ( float aX, float aY, float aZ, float aW );
        /// destructor.
        DLL ~Vector4();
        DLL const float* const GetVector4() const;
        DLL const float& GetX() const;
        DLL const float& GetY() const;
        DLL const float& GetZ() const;
        DLL const float& GetW() const;
    protected:
        float mVector[4];
    };
    DLL bool operator== ( const Vector4& aLhs, const Vector4& aRhs );
}
#endif

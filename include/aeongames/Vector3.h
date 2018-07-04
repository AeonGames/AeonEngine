/*
Copyright (C) 2015-2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VECTOR3_H
#define AEONGAMES_VECTOR3_H
/*! \file
    \brief Header for the 3D vector class.
    \author Rodrigo Hernandez.
    \copy 2015-2017
*/

#include "aeongames/Platform.h"
#include <cstdint>

namespace AeonGames
{
    class Plane;
    /*! \brief 3D vector class. */
    class Vector3
    {
    public:
        ///@brief Default constructor.
        DLL Vector3();
        /*! @brief Constructor
        @param aVector3 a float pointer or array containing vector data.
        @note This is the same format the values are internally stored and what Vector3::GetVector3 returns.
        */
        DLL Vector3 ( const float* const aVector );
        DLL Vector3 ( const void* const aVector, const uint32_t aStride );
        DLL Vector3 ( float aX, float aY, float aZ );
        /// destructor.
        DLL ~Vector3();
        DLL void Get ( float* aData ) const;
        DLL const float* const GetVector3() const;
        DLL const float& GetX() const;
        DLL const float& GetY() const;
        DLL const float& GetZ() const;
        DLL void SetVector3 ( const float* const aVector );
        DLL void SetVector3 ( const void* const aVector, const uint32_t aStride );
        DLL float GetLenghtSquared() const;
        DLL float GetLenght() const;
        DLL size_t GetMaxAxisIndex() const;
        DLL size_t GetMinAxisIndex() const;
        DLL float GetMaxAxisLenght() const;
        DLL float GetMinAxisLenght() const;
        DLL float GetDistanceToPlane ( const Plane& aPlane ) const;
        /*! \name Operators */
        //@{
        DLL Vector3& operator= ( const float* aLhs );
        DLL Vector3& operator-= ( const Vector3& aLhs );
        DLL Vector3& operator+= ( const Vector3& aLhs );
        DLL Vector3& operator*= ( const float aLhs );
        DLL Vector3& operator*= ( const Vector3& aLhs );
        DLL Vector3& operator/= ( const float aLhs );
        DLL float operator[] ( const uint32_t aIndex ) const;
        DLL float& operator [] ( const uint32_t aIndex );
        //@}
    protected:
        /// X,Y,Z
        float mVector[3];
    };
    DLL const Vector3 operator- ( const Vector3& aLhs, const Vector3& aRhs );
    DLL const Vector3 operator+ ( const Vector3& aLhs, const Vector3& aRhs );
    DLL const Vector3 operator* ( const Vector3& aLhs, const float aRhs );
    DLL const Vector3 operator/ ( const Vector3& aLhs, const float aRhs );
    DLL const Vector3 operator* ( const float aLhs, const Vector3& aRhs );
    DLL const Vector3 operator* ( const Vector3& aLhs, const Vector3& aRhs );
    DLL bool operator!= ( const Vector3& aLhs, const Vector3& aRhs );
    DLL bool operator== ( const Vector3& aLhs, const Vector3& aRhs );
    DLL const Vector3 Cross ( const Vector3& aLhs, const Vector3& aRhs );
    DLL const float Dot ( const Vector3& aLhs, const Vector3& aRhs );
    DLL const Vector3 Normalize ( const Vector3& aVector );
    DLL const Vector3 Abs ( const Vector3& aVector );
    DLL const Vector3 Spline ( const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, double interpolation );
}
#endif

/*
Copyright (C) 2015-2019,2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Vector3.hpp"
#include "aeongames/Plane.hpp"
#include "3DMath.h"

namespace AeonGames
{
    Vector3::Vector3()
    {
        memset ( mVector, 0, sizeof ( float ) * 3 );
    }

    Vector3::Vector3 ( const float* const aVector )
    {
        SetVector3 ( aVector );
    }

    void Vector3::SetVector3 ( const float* const aVector )
    {
        memcpy ( mVector, aVector, sizeof ( float ) * 3 );
    }

    Vector3::Vector3 ( const void* const aVector, const uint32_t aStride )
    {
        SetVector3 ( aVector, aStride );
    }

    void Vector3::SetVector3 ( const void* const aVector, const uint32_t aStride )
    {
        mVector[0] = * ( reinterpret_cast<const float*> ( aVector ) );
        mVector[1] = * ( reinterpret_cast<const float*> ( reinterpret_cast<const uint8_t*> ( aVector ) + ( aStride ) ) );
        mVector[2] = * ( reinterpret_cast<const float*> ( reinterpret_cast<const uint8_t*> ( aVector ) + ( aStride * 2 ) ) );
    }

    float Vector3::GetLenghtSquared() const
    {
        return Dot ( *this, *this );
    }

    float Vector3::GetLenght() const
    {
        return sqrtf ( GetLenghtSquared() );
    }

    size_t Vector3::GetMaxAxisIndex() const
    {
        return
            ( mVector[0] > mVector[1] ) ?
            ( ( mVector[0] > mVector[2] ) ? 0 : 2 ) :
            ( ( mVector[2] > mVector[1] ) ? 2 : 1 );
    }

    size_t Vector3::GetMinAxisIndex() const
    {
        return
            ( mVector[0] < mVector[1] ) ?
            ( ( mVector[0] < mVector[2] ) ? 0 : 2 ) :
            ( ( mVector[2] < mVector[1] ) ? 2 : 1 );
    }

    float Vector3::GetMaxAxisLenght() const
    {
        return mVector[GetMaxAxisIndex()];
    }

    float Vector3::GetMinAxisLenght() const
    {
        return mVector[GetMinAxisIndex()];
    }

    float Vector3::GetDistanceToPlane ( const Plane & aPlane ) const
    {
        return Dot ( aPlane.GetNormal(), *this ) - aPlane.GetDistance();
    }

    Vector3::Vector3 ( float aX, float aY, float aZ )
    {
        mVector[0] = aX;
        mVector[1] = aY;
        mVector[2] = aZ;
    }

    void Vector3::Get ( float* aData ) const
    {
        memcpy ( aData, mVector, sizeof ( float ) * 3 );
    }

    const float* const Vector3::GetVector3() const
    {
        return mVector;
    }

    const float& Vector3::GetX() const
    {
        return mVector[0];
    }

    const float& Vector3::GetY() const
    {
        return mVector[1];
    }


    const float& Vector3::GetZ() const
    {
        return mVector[2];
    }

    const Vector3 Cross ( const Vector3& aLhs, const Vector3& aRhs )
    {
        return Vector3 (
                   ( aLhs.GetY() * aRhs.GetZ() ) - ( aLhs.GetZ() * aRhs.GetY() ),
                   ( aLhs.GetZ() * aRhs.GetX() ) - ( aLhs.GetX() * aRhs.GetZ() ),
                   ( aLhs.GetX() * aRhs.GetY() ) - ( aLhs.GetY() * aRhs.GetX() ) );
    }

    float Vector3::operator [] ( const size_t aIndex ) const
    {
        assert ( aIndex < 3 );
        return mVector[aIndex];
    }

    float& Vector3::operator [] ( const size_t aIndex )
    {
        assert ( aIndex < 3 );
        return mVector[aIndex];
    }

    Vector3& Vector3::operator= ( const float* aLhs )
    {
        SetVector3 ( aLhs );
        return *this;
    }

    Vector3& Vector3::operator -= ( const Vector3& aLhs )
    {
        mVector[0] -= aLhs.mVector[0];
        mVector[1] -= aLhs.mVector[1];
        mVector[2] -= aLhs.mVector[2];
        return *this;
    }


    const Vector3 operator- ( const Vector3& aLhs, const Vector3& aRhs )
    {
        /*  Here Matrix4x4(lhs) *MAY* mean cast away constness
        rather tan create a temporary object in some compilers,
        we want the temporary, NOT the cast.*/
        return Vector3 ( aLhs ) -= aRhs;
    }

    Vector3& Vector3::operator += ( const Vector3& aLhs )
    {
        mVector[0] += aLhs.mVector[0];
        mVector[1] += aLhs.mVector[1];
        mVector[2] += aLhs.mVector[2];
        return *this;
    }
    const Vector3 operator+ ( const Vector3& aLhs, const Vector3& aRhs )
    {
        /*  Here Matrix4x4(lhs) *MAY* mean cast away constness
        rather tan create a temporary object in some compilers,
        we want the temporary, NOT the cast.*/
        return Vector3 ( aLhs ) += aRhs;
    }

    Vector3& Vector3::operator *= ( const float aLhs )
    {
        mVector[0] *= aLhs;
        mVector[1] *= aLhs;
        mVector[2] *= aLhs;
        return *this;
    }

    Vector3 & Vector3::operator*= ( const Vector3 & aLhs )
    {
        mVector[0] *= aLhs[0];
        mVector[1] *= aLhs[1];
        mVector[2] *= aLhs[2];
        return *this;
    }

    Vector3 & Vector3::operator/= ( const float aLhs )
    {
        mVector[0] /= aLhs;
        mVector[1] /= aLhs;
        mVector[2] /= aLhs;
        return *this;
    }

    const Vector3 operator* ( const Vector3& aLhs, const float aRhs )
    {
        /*  Here Matrix4x4(lhs) *MAY* mean cast away constness
        rather tan create a temporary object in some compilers,
        we want the temporary, NOT the cast.*/
        return Vector3 ( aLhs ) *= aRhs;
    }

    const Vector3 operator/ ( const Vector3 & aLhs, const float aRhs )
    {
        /*  Here Matrix4x4(lhs) *MAY* mean cast away constness
        rather tan create a temporary object in some compilers,
        we want the temporary, NOT the cast.*/
        return Vector3 ( aLhs ) /= aRhs;
    }

    const Vector3 operator* ( const float aLhs, const Vector3& aRhs )
    {
        /*  Here Matrix4x4(lhs) *MAY* mean cast away constness
        rather tan create a temporary object in some compilers,
        we want the temporary, NOT the cast.*/
        return Vector3 ( aRhs ) *= aLhs;
    }

    const Vector3 operator* ( const Vector3 & aLhs, const Vector3 & aRhs )
    {
        return Vector3 ( aRhs ) *= aLhs;
    }

    bool operator!= ( const Vector3& aLhs, const Vector3& aRhs )
    {
        return memcmp ( aLhs.GetVector3(), aRhs.GetVector3(), sizeof ( float ) * 3 ) != 0;
    }

    bool operator== ( const Vector3 & aLhs, const Vector3 & aRhs )
    {
        return memcmp ( aLhs.GetVector3(), aRhs.GetVector3(), sizeof ( float ) * 3 ) == 0;
    }

    const float Dot ( const Vector3& aLhs, const Vector3& aRhs )
    {
        return
            aLhs.GetX() * aRhs.GetX() +
            aLhs.GetY() * aRhs.GetY() +
            aLhs.GetZ() * aRhs.GetZ();
    }

    const Vector3 Normalize ( const Vector3& aVector )
    {
        float magnitude = sqrtf ( Dot ( aVector, aVector ) );
        assert ( magnitude != 0.0f );
        return Vector3 (
                   aVector.GetX() / magnitude,
                   aVector.GetY() / magnitude,
                   aVector.GetZ() / magnitude );
    }
    const Vector3 Abs ( const Vector3 & aVector )
    {
        return Vector3
        {
            std::abs ( aVector[0] ),
            std::abs ( aVector[1] ),
            std::abs ( aVector[2] )
        };
    }
    const Vector3 Spline ( const Vector3 & p0, const Vector3 & p1, const Vector3 & p2, const Vector3 & p3, double interpolation )
    {
        double i2 = interpolation * interpolation;
        double i3 = i2 * interpolation;
        double t0[3] =
        {
            ( p2[0] - p0[0] ) / 2.0,
            ( p2[1] - p0[1] ) / 2.0,
            ( p2[2] - p0[2] ) / 2.0
        };

        double t1[3] =
        {
            ( p3[0] - p1[0] ) / 2.0,
            ( p3[1] - p1[1] ) / 2.0,
            ( p3[2] - p1[2] ) / 2.0
        };
        return Vector3
        {
            static_cast<float> ( ( 2 * i3 - 3 * i2 + 1 ) * p1[0] + ( -2 * i3 + 3 * i2 ) * p2[0] + ( i3 - 2 * i2 + interpolation ) * t0[0] + ( i3 - i2 ) * t1[0] ),
            static_cast<float> ( ( 2 * i3 - 3 * i2 + 1 ) * p1[1] + ( -2 * i3 + 3 * i2 ) * p2[1] + ( i3 - 2 * i2 + interpolation ) * t0[1] + ( i3 - i2 ) * t1[1] ),
            static_cast<float> ( ( 2 * i3 - 3 * i2 + 1 ) * p1[2] + ( -2 * i3 + 3 * i2 ) * p2[2] + ( i3 - 2 * i2 + interpolation ) * t0[2] + ( i3 - i2 ) * t1[2] )
        };
    }
}

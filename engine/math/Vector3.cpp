/*
Copyright 2015-2016 Rodrigo Jose Hernandez Cordoba

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
#include "Vector3.h"
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

    Vector3::Vector3 ( float aX, float aY, float aZ )
    {
        mVector[0] = aX;
        mVector[1] = aY;
        mVector[2] = aZ;
    }

    Vector3::~Vector3()
    {
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

    float Vector3::operator [] ( const uint32_t aIndex ) const
    {
        assert ( aIndex < 3 );
        return mVector[aIndex];
    }

    float& Vector3::operator [] ( const uint32_t aIndex )
    {
        assert ( aIndex < 3 );
        return mVector[aIndex];
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

    const Vector3 operator* ( const Vector3& aLhs, const float aRhs )
    {
        /*  Here Matrix4x4(lhs) *MAY* mean cast away constness
        rather tan create a temporary object in some compilers,
        we want the temporary, NOT the cast.*/
        return Vector3 ( aLhs ) *= aRhs;
    }

    const Vector3 operator* ( const float aLhs, const Vector3& aRhs )
    {
        /*  Here Matrix4x4(lhs) *MAY* mean cast away constness
        rather tan create a temporary object in some compilers,
        we want the temporary, NOT the cast.*/
        return Vector3 ( aRhs ) *= aLhs;
    }

    bool operator!= ( const Vector3& aLhs, const Vector3& aRhs )
    {
        return memcmp ( aLhs.GetVector3(), aRhs.GetVector3(), sizeof ( float ) * 3 ) != 0;
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
}

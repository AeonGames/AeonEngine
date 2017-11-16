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
#define _USE_MATH_DEFINES
#include "aeongames/Quaternion.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/Vector3.h"
#include <cassert>
#include <cmath>

namespace AeonGames
{
    Quaternion::Quaternion()
    {
    }
    Quaternion::Quaternion ( float w, float x, float y, float z ) :
        mQuaternion{w, x, y, z}
    {
    }

    Quaternion::~Quaternion()
        = default;
    Matrix4x4 Quaternion::GetMatrix4x4() const
    {
        return Matrix4x4
        {
            1.0f - 2.0f * ( mQuaternion[2] * mQuaternion[2] + mQuaternion[3] * mQuaternion[3] ),
            2.0f * ( mQuaternion[1] * mQuaternion[2] + mQuaternion[3] * mQuaternion[0] ),
            2.0f * ( mQuaternion[1] * mQuaternion[3] - mQuaternion[2] * mQuaternion[0] ),
            0.0f,
            // Second row
            2.0f * ( mQuaternion[1] * mQuaternion[2] - mQuaternion[3] * mQuaternion[0] ),
            1.0f - 2.0f * ( mQuaternion[1] * mQuaternion[1] + mQuaternion[3] * mQuaternion[3] ),
            2.0f * ( mQuaternion[3] * mQuaternion[2] + mQuaternion[1] * mQuaternion[0] ),
            0.0f,
            // Third row
            2.0f * ( mQuaternion[1] * mQuaternion[3] + mQuaternion[2] * mQuaternion[0] ),
            2.0f * ( mQuaternion[2] * mQuaternion[3] - mQuaternion[1] * mQuaternion[0] ),
            1.0f - 2.0f * ( mQuaternion[1] * mQuaternion[1] + mQuaternion[2] * mQuaternion[2] ),
            0.0f,
            0, 0, 0, 1};
    }

    Quaternion& Quaternion::operator*= ( const Quaternion& lhs )
    {
        float local[4] { mQuaternion[0], mQuaternion[1], mQuaternion[2], mQuaternion[3] };
        mQuaternion[0] = ( local[0] * lhs.mQuaternion[0] - local[1] * lhs.mQuaternion[1] - local[2] * lhs.mQuaternion[2] - local[3] * lhs.mQuaternion[3] );
        mQuaternion[1] = ( local[0] * lhs.mQuaternion[1] + local[1] * lhs.mQuaternion[0] + local[2] * lhs.mQuaternion[3] - local[3] * lhs.mQuaternion[2] );
        mQuaternion[2] = ( local[0] * lhs.mQuaternion[2] - local[1] * lhs.mQuaternion[3] + local[2] * lhs.mQuaternion[0] + local[3] * lhs.mQuaternion[1] );
        mQuaternion[3] = ( local[0] * lhs.mQuaternion[3] + local[1] * lhs.mQuaternion[2] - local[2] * lhs.mQuaternion[1] + local[3] * lhs.mQuaternion[0] );
        return *this;
    }

    const Quaternion operator* ( const Quaternion& lhs, const Quaternion& rhs )
    {
        return Quaternion ( lhs ) *= rhs;
    }

    const Vector3 operator* ( const Quaternion & lhs, const Vector3 & rhs )
    {

        float t1 = ( -lhs[1] * rhs[0] - lhs[2] * rhs[1] - lhs[3] * rhs[2] );
        float t2 = ( lhs[0] * rhs[0] + lhs[2] * rhs[2] - lhs[3] * rhs[1] );
        float t3 = ( lhs[0] * rhs[1] + lhs[3] * rhs[0] - lhs[1] * rhs[2] );
        float t4 = ( lhs[0] * rhs[2] + lhs[1] * rhs[1] - lhs[2] * rhs[0] );
        return Vector3
        {
            t1 * -lhs[1] + t2 * lhs[0] + t3 * -lhs[3] - t4 * -lhs[2],
            t1 * -lhs[2] + t3 * lhs[0] + t4 * -lhs[1] - t2 * -lhs[3],
            t1 * -lhs[3] + t4 * lhs[0] + t2 * -lhs[2] - t3 * -lhs[1]
        };
    }

    bool operator== ( const Quaternion & lhs, const Quaternion & rhs )
    {
        return
            lhs[0] == rhs[0] &&
            lhs[1] == rhs[1] &&
            lhs[2] == rhs[2] &&
            lhs[3] == rhs[3];
    }

    const Quaternion LerpQuats ( const Quaternion & q1, const Quaternion & q2, double interpolation )
    {
        if ( interpolation <= 0.0f )
        {
            return Quaternion ( q1 );
        }
        else if ( interpolation >= 1.0f )
        {
            return Quaternion ( q2 );
        }
        return Quaternion
        {
            static_cast<float> ( ( q1[0] * ( 1.0 - interpolation ) ) + ( q2[0] * interpolation ) ),
            static_cast<float> ( ( q1[1] * ( 1.0 - interpolation ) ) + ( q2[1] * interpolation ) ),
            static_cast<float> ( ( q1[2] * ( 1.0 - interpolation ) ) + ( q2[2] * interpolation ) ),
            static_cast<float> ( ( q1[3] * ( 1.0 - interpolation ) ) + ( q2[3] * interpolation ) )
        };
    }

    const Quaternion NlerpQuats ( const Quaternion & q1, const Quaternion & q2, double interpolation )
    {
        return Quaternion ( LerpQuats ( q1, q2, interpolation ) ).Normalize();
    }

    const Quaternion SlerpQuats ( const Quaternion & q1, const Quaternion & q2, float interpolation )
    {
        {
            if ( interpolation <= 0.0f )
            {
                return Quaternion ( q1 );
            }
            else if ( interpolation >= 1.0f )
            {
                return Quaternion ( q2 );
            }
            float dot = ( q1[0] * q2[0] ) + ( q1[1] * q2[1] ) + ( q1[2] * q2[2] ) + ( q1[3] * q2[3] );
            float sign = 1.0f;
            if ( fabs ( dot ) > 0.9999f )
            {
                return Quaternion ( q1 );
            }
            else if ( dot < 0.0f )
            {
                dot = -dot;
                sign = -1.0;
            }
            float   theta = acosf ( dot );
            float   sinT = 1.0f / sinf ( theta );
            float   newFactor = sinf ( interpolation * theta ) * sinT;
            float   invFactor = sinf ( ( 1.0f - interpolation ) * theta ) * sinT;

            return Quaternion
            {
                invFactor * q1[0] + newFactor * q2[0] * sign,
                invFactor * q1[1] + newFactor * q2[1] * sign,
                invFactor * q1[2] + newFactor * q2[2] * sign,
                invFactor * q1[3] + newFactor * q2[3] * sign
            };
        }
    }

    float Quaternion::operator [] ( const size_t aIndex ) const
    {
        assert ( aIndex < 4 );
        return mQuaternion[aIndex];
    }

    float& Quaternion::operator [] ( const size_t aIndex )
    {
        assert ( aIndex < 4 );
        return mQuaternion[aIndex];
    }

    const Quaternion Quaternion::GetFromAxisAngle ( float angle, float x, float y, float z )
    {
        float radians = ( angle / 180.0f ) * static_cast<float> ( M_PI );
        float result = sinf ( radians / 2.0f );
        return Quaternion
        {
            cosf ( radians / 2.0f ),
            x * result,
            y * result,
            z * result
        };
    }
    Quaternion & Quaternion::Normalize()
    {
        float length = sqrtf (
                           ( mQuaternion[0] * mQuaternion[0] ) +
                           ( mQuaternion[1] * mQuaternion[1] ) +
                           ( mQuaternion[2] * mQuaternion[2] ) +
                           ( mQuaternion[3] * mQuaternion[3] ) );
        // do nothing if length = 0
        if ( length )
        {
            float oneoverlength = 1.0f / length;
            mQuaternion[0] *= oneoverlength;
            mQuaternion[1] *= oneoverlength;
            mQuaternion[2] *= oneoverlength;
            mQuaternion[3] *= oneoverlength;
        }
        return *this;
    }
}

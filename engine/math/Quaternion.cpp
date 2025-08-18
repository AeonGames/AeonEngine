/*
Copyright (C) 2017-2019,2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Quaternion.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Matrix3x3.hpp"
#include "aeongames/Vector3.hpp"
#include <cassert>
#include <cmath>
#include <cstring>

namespace AeonGames
{
    Quaternion::Quaternion()
        = default;
    Quaternion::Quaternion ( float w, float x, float y, float z ) :
        mQuaternion{w, x, y, z}
    {
    }

    Quaternion::Quaternion ( const float* aData ) :
        mQuaternion{aData[0], aData[1], aData[2], aData[3]}
    {
    }

    Quaternion::~Quaternion()
        = default;
    Matrix4x4 Quaternion::GetMatrix4x4() const
    {
        // Products
        float p1 = mQuaternion[0] * mQuaternion[1];
        float p2 = mQuaternion[0] * mQuaternion[2];
        float p3 = mQuaternion[0] * mQuaternion[3];

        float p4 = mQuaternion[1] * mQuaternion[1];
        float p5 = mQuaternion[1] * mQuaternion[2];
        float p6 = mQuaternion[1] * mQuaternion[3];

        float p7 = mQuaternion[2] * mQuaternion[2];
        float p8 = mQuaternion[2] * mQuaternion[3];

        float p9 = mQuaternion[3] * mQuaternion[3];

        return Matrix4x4
        {
            // First row
            1.0f - 2.0f * ( p7 + p9 ),
            2.0f * ( p5 + p3 ),
            2.0f * ( p6 - p2 ),
            0.0f,
            // Second row
            2.0f * ( p5 - p3 ),
            1.0f - 2.0f * ( p4 + p9 ),
            2.0f * ( p8 + p1 ),
            0.0f,
            // Third row
            2.0f * ( p6 + p2 ),
            2.0f * ( p8 - p1 ),
            1.0f - 2.0f * ( p4 + p7 ),
            0.0f,
            // Fourth row
            0, 0, 0, 1};
    }

    Matrix3x3 Quaternion::GetMatrix3x3() const
    {
        // Products
        float p1 = mQuaternion[0] * mQuaternion[1];
        float p2 = mQuaternion[0] * mQuaternion[2];
        float p3 = mQuaternion[0] * mQuaternion[3];

        float p4 = mQuaternion[1] * mQuaternion[1];
        float p5 = mQuaternion[1] * mQuaternion[2];
        float p6 = mQuaternion[1] * mQuaternion[3];

        float p7 = mQuaternion[2] * mQuaternion[2];
        float p8 = mQuaternion[2] * mQuaternion[3];

        float p9 = mQuaternion[3] * mQuaternion[3];

        return Matrix3x3
        {
            // First row
            1.0f - 2.0f * ( p7 + p9 ),
            2.0f * ( p5 + p3 ),
            2.0f * ( p6 - p2 ),
            // Second row
            2.0f * ( p5 - p3 ),
            1.0f - 2.0f * ( p4 + p9 ),
            2.0f * ( p8 + p1 ),
            // Third row
            2.0f * ( p6 + p2 ),
            2.0f * ( p8 - p1 ),
            1.0f - 2.0f * ( p4 + p7 ),
        };
    }

    Quaternion& Quaternion::operator= ( const float* aLhs )
    {
        memcpy ( mQuaternion, aLhs, sizeof ( float ) * 4 );
        return *this;
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
            if ( std::fabs ( dot ) > 0.9999f )
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
        } .Normalize();
    }

    const Quaternion Quaternion::GetFromEuler ( const Vector3& aEuler )
    {
        return Quaternion{} .SetEuler ( aEuler );
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

    void Quaternion::Get ( float* aData ) const
    {
        memcpy ( aData, mQuaternion, sizeof ( float ) * 4 );
    }

    Vector3 Quaternion::GetEuler() const
    {
        double sqw = mQuaternion[0] * mQuaternion[0];
        double sqx = mQuaternion[1] * mQuaternion[1];
        double sqy = mQuaternion[2] * mQuaternion[2];
        double sqz = mQuaternion[3] * mQuaternion[3];
        double unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
        double test = mQuaternion[1] * mQuaternion[2] + mQuaternion[3] * mQuaternion[0];
        if ( test > 0.499 * unit )
        {
            // singularity at north pole
            return Vector3 (
                       0.0f,
                       static_cast<float> ( ( 2.0  * std::atan2 ( mQuaternion[1], mQuaternion[0] ) ) * ( 180.0 / M_PI ) ),
                       static_cast<float> ( ( M_PI / 2 ) * ( 180 / M_PI ) ) );
        }
        if ( test < -0.499 * unit )
        {
            // singularity at south pole
            return Vector3 (
                       0.0f,
                       static_cast<float> ( ( -2 * std::atan2 ( mQuaternion[1], mQuaternion[0] ) ) * ( 180 / M_PI ) ),
                       static_cast<float> ( ( -M_PI / 2 ) * ( 180 / M_PI ) ) );
        }
        return Vector3 (
                   static_cast<float> ( std::atan2 ( 2 * mQuaternion[1] * mQuaternion[0] - 2 * mQuaternion[2] * mQuaternion[3], -sqx + sqy - sqz + sqw ) * ( 180 / M_PI ) ),
                   static_cast<float> ( std::atan2 ( 2 * mQuaternion[2] * mQuaternion[0] - 2 * mQuaternion[1] * mQuaternion[3], sqx - sqy - sqz + sqw ) * ( 180 / M_PI ) ),
                   static_cast<float> ( std::asin ( 2 * test / unit ) * ( 180 / M_PI ) ) );
    }

    Quaternion& Quaternion::SetEuler ( const Vector3& aEuler )
    {
        // Euler must be given in Degrees
        double rad_pitch_over_2 = ( ( M_PI / 180 ) * aEuler[0] ) / 2;
        double rad_roll_over_2 = ( ( M_PI / 180 ) * aEuler[1] ) / 2;
        double rad_yaw_over_2 = ( ( M_PI / 180 ) * aEuler[2] ) / 2;
        double c3 = std::cos ( rad_pitch_over_2 );
        double s3 = std::sin ( rad_pitch_over_2 );
        double c1 = std::cos ( rad_roll_over_2 );
        double s1 = std::sin ( rad_roll_over_2 );
        double c2 = std::cos ( rad_yaw_over_2 );
        double s2 = std::sin ( rad_yaw_over_2 );
        double c1c2 = c1 * c2;
        double s1s2 = s1 * s2;
        mQuaternion[0] = static_cast<float> ( c1c2 * c3 - s1s2 * s3 );
        mQuaternion[1] = static_cast<float> ( c1c2 * s3 + s1s2 * c3 );
        mQuaternion[2] = static_cast<float> ( s1 * c2 * c3 + c1 * s2 * s3 );
        mQuaternion[3] = static_cast<float> ( c1 * s2 * c3 - s1 * c2 * s3 );
        Normalize();
        return *this;
    }
}

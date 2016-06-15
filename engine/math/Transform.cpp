/*
Copyright 2014-2016 Rodrigo Jose Hernandez Cordoba

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

#include "aeongames/Transform.h"
#include "3DMath.h"

namespace AeonGames
{
    Transform::Transform()
    {
        // set all to identities.
        srt[0] = srt[1] = srt[2] = srt[3] = 1.0f;
        srt[4] = srt[5] = srt[6] = srt[7] = srt[8] = srt[9] = 0.0f;
    }

    Transform::Transform (
        float aTx, float aTy, float aTz,
        float aRw, float aRx, float aRy, float aRz,
        float aSx, float aSy, float aSz )
    {
        srt[0] = aSx;
        srt[1] = aSy;
        srt[2] = aSz;
        srt[3] = aRw;
        srt[4] = aRx;
        srt[5] = aRy;
        srt[6] = aRz;
        srt[7] = aTx;
        srt[8] = aTy;
        srt[9] = aTz;
    }

    Transform::Transform ( const float* const aSRT )
    {
        memcpy ( srt, aSRT, sizeof ( float ) * 10 );
    }

    Transform::~Transform()
    {
    }

    const float* const Transform::GetScale() const
    {
        return srt;
    }
    const float* const Transform::GetRotation() const
    {
        return srt + 3;
    }
    const float* const Transform::GetTranslation() const
    {
        return srt + 7;
    }
    const float* const Transform::GetTransform() const
    {
        return srt;
    }

    float Transform::GetScaleX() const
    {
        return srt[0];
    }
    float Transform::GetScaleY() const
    {
        return srt[1];
    }
    float Transform::GetScaleZ() const
    {
        return srt[2];
    }
    float Transform::GetRotationW() const
    {
        return srt[3];
    }
    float Transform::GetRotationX() const
    {
        return srt[4];
    }
    float Transform::GetRotationY() const
    {
        return srt[5];
    }
    float Transform::GetRotationZ() const
    {
        return srt[6];
    }
    float Transform::GetTranslationX() const
    {
        return srt[7];
    }
    float Transform::GetTranslationY() const
    {
        return srt[8];
    }
    float Transform::GetTranslationZ() const
    {
        return srt[9];
    }

    void Transform::GetTranslation ( float& x, float& y, float& z ) const
    {
        x = srt[7];
        y = srt[8];
        z = srt[9];
    }

    void Transform::GetTranslation ( float* v ) const
    {
        memcpy ( v, srt + 7, sizeof ( float ) * 3 );
    }

    void Transform::SetScale ( float x, float y, float z )
    {
        srt[0] = x;
        srt[1] = y;
        srt[2] = z;
    }

    void Transform::SetScaleX ( float v )
    {
        srt[0] = v;
    }
    void Transform::SetScaleY ( float v )
    {
        srt[1] = v;
    }
    void Transform::SetScaleZ ( float v )
    {
        srt[2] = v;
    }

    void Transform::SetRotation ( float w, float x, float y, float z )
    {
        srt[3] = w;
        srt[4] = x;
        srt[5] = y;
        srt[6] = z;
    }

    void Transform::SetRotationW ( float v )
    {
        srt[3] = v;
    }
    void Transform::SetRotationX ( float v )
    {
        srt[4] = v;
    }
    void Transform::SetRotationY ( float v )
    {
        srt[5] = v;
    }
    void Transform::SetRotationZ ( float v )
    {
        srt[6] = v;
    }

    void Transform::SetTranslation ( float x, float y, float z )
    {
        srt[7] = x;
        srt[8] = y;
        srt[9] = z;
    }

    void Transform::SetTranslationX ( float v )
    {
        srt[7] = v;
    }
    void Transform::SetTranslationY ( float v )
    {
        srt[8] = v;
    }
    void Transform::SetTranslationZ ( float v )
    {
        srt[9] = v;
    }


    void Transform::SetScale ( float* v )
    {
        memcpy ( srt, v, sizeof ( float ) * 3 );
    }
    void Transform::SetRotation ( float* v )
    {
        memcpy ( srt + 2, v, sizeof ( float ) * 4 );
    }
    void Transform::SetTranslation ( float* v )
    {
        memcpy ( srt + 7, v, sizeof ( float ) * 3 );
    }


    void Transform::RotateObjectSpace ( float angle, float x, float y, float z )
    {
        float qp[4];
        AngleAxisToQuat ( angle, x, y, z, qp );
        MultQuats ( srt + 3, qp, srt + 3 );
    }

    void Transform::RotateInertialSpace ( float angle, float x, float y, float z )
    {
        float qp[4];
        AngleAxisToQuat ( angle, x, y, z, qp );
        MultQuats ( qp, srt + 3, srt + 3 );
    }

    void Transform::Move ( float x, float y, float z )
    {
        srt[7] += x;
        srt[8] += y;
        srt[9] += z;
    }

    void Transform::ResetRotation()
    {
        srt[3] = 1.0f;
        srt[4] = 0.0f;
        srt[5] = 0.0f;
        srt[6] = 0.0f;
    }

    float* Transform::GetMatrix ( float* M ) const
    {
        ///\todo This could be cached.
        return GetMatrixFromSRT ( srt, M );
    }

    float* Transform::GetInvertedMatrix ( float* M )
    {
        ///\todo This could be cached.
        return GetInvertedMatrixFromSRT ( srt, M );
    }

    void Transform::MoveInObjectSpace ( float x, float y, float z )
    {
        float v[4] = {x, y, z, 0.0f};
        RotateVectorByQuat ( srt + 3, v, v );
        srt[7] += v[0];
        srt[8] += v[1];
        srt[9] += v[2];
    }

    Transform& Transform::Invert()
    {
        InvertSRT ( srt, srt );
        return *this;
    }

    const Transform Transform::GetInverted()
    {
        return Transform ( *this ).Invert();
    }

    Transform& Transform::operator *= ( const Transform& lhs )
    {
        // Item 22 from MEC++
        MultSRTs ( srt, lhs.srt, srt );
        return *this;
    }

    const Transform operator* ( const Transform& lhs, const Transform& rhs )
    {
        /*  Here Transform(lhs) *MAY* mean cast away constness
            rather tan create a temporary object in some compilers,
            we want the temporary, NOT the cast.*/
        return Transform ( lhs ) *= rhs;
    }

    const bool operator== ( const Transform& lhs, const Transform& rhs )
    {
        return memcmp ( lhs.GetTransform(), rhs.GetTransform(), sizeof ( float ) * 10 ) == 0;
    }
}

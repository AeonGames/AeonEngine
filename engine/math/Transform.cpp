/*
Copyright (C) 2014-2017 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AABB.h"
#include "aeongames/Matrix4x4.h"
#include "3DMath.h"

namespace AeonGames
{
    Transform::Transform()
    {
    }

    Transform::Transform (
        float aTx, float aTy, float aTz,
        float aRw, float aRx, float aRy, float aRz,
        float aSx, float aSy, float aSz ) :
        mScale{ aSx, aSy, aSz },
        mRotation{ aRw, aRx, aRy, aRz },
        mTranslation{ aTx, aTy, aTz }
    {
    }

    Transform::Transform ( const float* const aSRT ) :
        mScale{ aSRT[0], aSRT[1], aSRT[2] },
        mRotation{ aSRT[3], aSRT[4], aSRT[5], aSRT[6] },
        mTranslation{ aSRT[7], aSRT[8], aSRT[9] }
    {
    }

    Transform::Transform ( const Vector3 & aScale, const Quaternion & aRotation, const Vector3 & aTranslation ) :
        mScale{ aScale }, mRotation{ aRotation }, mTranslation{aTranslation}
    {
    }

    Transform::~Transform()
        = default;

    const Vector3& Transform::GetScale() const
    {
        return mScale;
    }
    const Quaternion& Transform::GetRotation() const
    {
        return mRotation;
    }
    const Vector3& Transform::GetTranslation() const
    {
        return mTranslation;
    }

    void Transform::SetScale ( float x, float y, float z )
    {
        mScale[0] = x;
        mScale[1] = y;
        mScale[2] = z;
    }

    void Transform::SetScaleX ( float v )
    {
        mScale[0] = v;
    }
    void Transform::SetScaleY ( float v )
    {
        mScale[1] = v;
    }
    void Transform::SetScaleZ ( float v )
    {
        mScale[2] = v;
    }

    void Transform::SetRotation ( float w, float x, float y, float z )
    {
        mRotation[0] = w;
        mRotation[1] = x;
        mRotation[2] = y;
        mRotation[3] = z;
    }

    void Transform::SetRotationW ( float v )
    {
        mRotation[0] = v;
    }
    void Transform::SetRotationX ( float v )
    {
        mRotation[1] = v;
    }
    void Transform::SetRotationY ( float v )
    {
        mRotation[2] = v;
    }
    void Transform::SetRotationZ ( float v )
    {
        mRotation[3] = v;
    }

    void Transform::SetTranslation ( float x, float y, float z )
    {
        mTranslation[0] = x;
        mTranslation[1] = y;
        mTranslation[2] = z;
    }

    void Transform::SetTranslationX ( float v )
    {
        mTranslation[0] = v;
    }
    void Transform::SetTranslationY ( float v )
    {
        mTranslation[1] = v;
    }
    void Transform::SetTranslationZ ( float v )
    {
        mTranslation[2] = v;
    }


    void Transform::SetScale ( float* v )
    {
        mScale[0] = v[0];
        mScale[1] = v[1];
        mScale[2] = v[2];
    }
    void Transform::SetRotation ( float* v )
    {
        mRotation[0] = v[0];
        mRotation[1] = v[1];
        mRotation[2] = v[2];
        mRotation[3] = v[3];
    }
    void Transform::SetTranslation ( float* v )
    {
        mTranslation[0] = v[0];
        mTranslation[1] = v[1];
        mTranslation[2] = v[2];
    }

    void Transform::RotateObjectSpace ( float angle, float x, float y, float z )
    {
        mRotation *= Quaternion::GetFromAxisAngle ( angle, x, y, z );
    }

    void Transform::RotateInertialSpace ( float angle, float x, float y, float z )
    {
        mRotation = Quaternion::GetFromAxisAngle ( angle, x, y, z ) * mRotation;
    }

    void Transform::Move ( float x, float y, float z )
    {
        mTranslation[0] += x;
        mTranslation[1] += y;
        mTranslation[2] += z;
    }

    void Transform::ResetRotation()
    {
        mRotation[0] = 1.0f;
        mRotation[1] = 0.0f;
        mRotation[2] = 0.0f;
        mRotation[3] = 0.0f;
    }

    const Matrix4x4 Transform::GetMatrix () const
    {
        Matrix4x4 rotation = mRotation.GetMatrix4x4();
        return Matrix4x4
        {
            // Simplified 3x3 scale matrix multiplication
            rotation[0] * mScale[0],
            rotation[1] * mScale[0],
            rotation[2] * mScale[0],
            0,

            rotation[4] * mScale[1],
            rotation[5] * mScale[1],
            rotation[6] * mScale[1],
            0,

            rotation[8] * mScale[2],
            rotation[9] * mScale[2],
            rotation[10] * mScale[2],
            0,
            // Simplified translation multiplication
            mTranslation[0],
            mTranslation[1],
            mTranslation[2],
            1,
        };
    }

    const Matrix4x4 Transform::GetInvertedMatrix () const
    {
        return GetInverted().GetMatrix();
    }

    void Transform::MoveInObjectSpace ( float x, float y, float z )
    {
        mTranslation += mRotation * Vector3 ( x, y, z );
    }

    Transform& Transform::Invert()
    {
        mScale[0] = 1.0f / mScale[0];
        mScale[1] = 1.0f / mScale[1];
        mScale[2] = 1.0f / mScale[2];
        //mRotation[0] = mRotation[0]; // Stays the same
        mRotation[1] = -mRotation[1];
        mRotation[2] = -mRotation[2];
        mRotation[3] = -mRotation[3];
        mTranslation *= -1;
        mTranslation = mRotation * mTranslation;
        return *this;
    }

    const Transform Transform::GetInverted() const
    {
        return Transform ( *this ).Invert();
    }

    Transform& Transform::operator *= ( const Transform& lhs )
    {
        // Item 22 from MEC++
        // Scale
        mScale *= lhs.GetScale();
        // Rotation
        mRotation *= lhs.GetRotation();
        // Translation
        mTranslation += mRotation * lhs.GetTranslation();
        return *this;
    }

    const Transform operator* ( const Transform& lhs, const Transform& rhs )
    {
        /*  Here Transform(lhs) *MAY* mean cast away constness
            rather tan create a temporary object in some compilers,
            we want the temporary, NOT the cast.*/
        return Transform ( lhs ) *= rhs;
    }

    const AABB operator* ( const Transform & lhs, const AABB & rhs )
    {
        ///@note Based on Real Time Collision Detection 4.2.6
        return AABB
        {
            lhs.GetTranslation() + ( lhs.GetRotation() *rhs.GetCenter() ),
            Abs ( lhs.GetRotation() * ( lhs.GetScale() * rhs.GetRadii() ) )
        };
    }

    const bool operator== ( const Transform& lhs, const Transform& rhs )
    {
        return
            lhs.GetScale() == rhs.GetScale() &&
            lhs.GetRotation() == rhs.GetRotation() &&
            lhs.GetTranslation() == rhs.GetTranslation();
    }
    const Transform Interpolate ( const Transform & aTransform0, const Transform & aTransform1, const Transform & aTransform2, const Transform & aTransform3, double aInterpolation )
    {
        return Transform
        {
            Spline ( aTransform0.GetScale(), aTransform1.GetScale(), aTransform2.GetScale(), aTransform3.GetScale(), aInterpolation ),
            NlerpQuats ( aTransform1.GetRotation(), aTransform1.GetRotation(), aInterpolation ),
            Spline ( aTransform0.GetTranslation(), aTransform1.GetTranslation(), aTransform2.GetTranslation(), aTransform3.GetTranslation(), aInterpolation )
        };
    }
}

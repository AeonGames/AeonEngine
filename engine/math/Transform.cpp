/*
Copyright (C) 2014-2018 Rodrigo Jose Hernandez Cordoba

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
        = default;

    Transform::Transform ( const Vector3 & aScale, const Quaternion & aRotation, const Vector3 & aTranslation ) :
        mScale{ aScale }, mRotation{ aRotation }, mTranslation{aTranslation}
    {
    }

    Transform::Transform ( const Vector3 & aScale, const Vector3 & aRotation, const Vector3 & aTranslation ) :
        mScale{ aScale }, mRotation{ Quaternion::GetFromEuler ( aRotation ) }, mTranslation{aTranslation}
    {
    }

    Transform::Transform ( const float* aData ) :
        mScale{ aData }, mRotation{ aData + 3 }, mTranslation{aData + 7}
    {
    }

    void Transform::Get ( float* aData ) const
    {
        mScale.Get ( aData );
        mRotation.Get ( aData + 3 );
        mTranslation.Get ( aData + 7 );
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

    void Transform::SetScale ( const Vector3& aScale )
    {
        mScale = aScale;
    }

    void Transform::SetRotation ( const Quaternion& aRotation )
    {
        mRotation = aRotation;
    }

    void Transform::SetTranslation ( const Vector3& aTranslation )
    {
        mTranslation = aTranslation;
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
        mTranslation = mRotation * ( mTranslation * -1 );
        return *this;
    }

    const Transform Transform::GetInverted() const
    {
        return Transform ( *this ).Invert();
    }

    Transform& Transform::operator *= ( const Transform& lhs )
    {
        // Item 22 from MEC++
        // Translation
        mTranslation  = ( mRotation * lhs.GetTranslation() ) + mTranslation;
        // Rotation
        mRotation *= lhs.GetRotation();
        // Scale
        mScale *= lhs.GetScale();
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

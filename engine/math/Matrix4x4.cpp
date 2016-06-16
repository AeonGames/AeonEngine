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
#include "aeongames/Matrix4x4.h"
#include "aeongames/Transform.h"
#include "3DMath.h"

namespace AeonGames
{
    Matrix4x4::Matrix4x4()
    {
        SetIdentityMatrix4x4 ( mMatrix );
    }

    Matrix4x4::Matrix4x4 ( const Transform& aTransform )
    {
        aTransform.GetMatrix ( mMatrix );
    }

    Matrix4x4::Matrix4x4 ( const float* const aMatrix4x4 )
    {
        memcpy ( mMatrix, aMatrix4x4, sizeof ( float ) * 16 );
    }

    Matrix4x4::~Matrix4x4()
    {
    }

    const float* const Matrix4x4::GetMatrix4x4() const
    {
        return mMatrix;
    }

    Matrix4x4& Matrix4x4::Invert()
    {
        InvertMatrix ( mMatrix, mMatrix );
        return *this;
    }

    const Matrix4x4 Matrix4x4::GetInvertedMatrix4x4()
    {
        return Matrix4x4 ( *this ).Invert();
    }

    void Matrix4x4::Frustum ( float aLeft, float aRight, float aBottom, float aTop, float aNear, float aFar )
    {
        // glFrustum, +Z up, +Y forward
        // X
        mMatrix[0] = ( 2 * aNear ) / ( aRight - aLeft );
        mMatrix[1] = 0.0f;
        mMatrix[2] = 0.0f;
        mMatrix[3] = 0.0f;

        // Y
        mMatrix[4] = - ( ( aRight + aLeft ) / ( aRight - aLeft ) );
        mMatrix[5] = - ( ( aTop + aBottom ) / ( aTop - aBottom ) );
        mMatrix[6] = ( ( aFar + aNear ) / ( aFar - aNear ) );
        mMatrix[7] = 1.0f;

        // Z
        mMatrix[8] = 0.0f;
        mMatrix[9] = ( 2 * aNear ) / ( aTop - aBottom );
        mMatrix[10] = 0.0f;
        mMatrix[11] = 0.0f;

        // Pos
        mMatrix[12] = 0.0f;
        mMatrix[13] = 0.0f;
        mMatrix[14] = - ( ( 2 * aFar * aNear ) / ( aFar - aNear ) );
        mMatrix[15] = 0.0f;
    }

    void Matrix4x4::Ortho ( float aLeft, float aRight, float aBottom, float aTop, float aNear, float aFar )
    {
        ///\note This function is yet to be tested for correctness.
        // glOrtho, +Z up, +Y forward

        // X
        mMatrix[0] = ( 2.0f / ( aRight - aLeft ) );
        mMatrix[1] = 0.0f;
        mMatrix[2] = 0.0f;
        mMatrix[3] = 0.0f;

        // Y
        mMatrix[4] = 0.0f;
        mMatrix[5] = 0.0f;
        mMatrix[6] = - ( 2.0f / ( aFar - aNear ) );
        mMatrix[7] = 0.0f;

        // Z
        mMatrix[8] = 0.0f;
        mMatrix[9] = ( 2.0f / ( aTop - aBottom ) );
        mMatrix[10] = 0.0f;
        mMatrix[11] = 0.0f;

        // Pos
        mMatrix[12] = - ( ( aRight + aLeft ) / ( aRight - aLeft ) );
        mMatrix[13] = - ( ( aTop + aBottom ) / ( aTop - aBottom ) );
        mMatrix[14] = - ( ( aFar + aNear ) / ( aFar - aNear ) );
        mMatrix[15] = 1.0f;
    }

    void Matrix4x4::Perspective ( float aFieldOfVision, float aAspect, float aNear, float aFar )
    {
        float fH = tan ( aFieldOfVision / 360 * PI ) * aNear;
        float fW = fH * aAspect;
        Frustum ( -fW, fW, -fH, fH, aNear, aFar );
    }

    Matrix4x4& Matrix4x4::operator *= ( const Matrix4x4& lhs )
    {
        // Item 22 from MEC++
        Multiply4x4Matrix ( mMatrix, lhs.mMatrix, mMatrix );
        return *this;
    }

    const Matrix4x4 operator* ( const Matrix4x4& lhs, const Matrix4x4& rhs )
    {
        /*  Here Matrix4x4(lhs) *MAY* mean cast away constness
            rather tan create a temporary object in some compilers,
            we want the temporary, NOT the cast.*/
        return Matrix4x4 ( lhs ) *= rhs;
    }

    const bool operator== ( const Matrix4x4& lhs, const Matrix4x4& rhs )
    {
        return memcmp ( lhs.GetMatrix4x4(), rhs.GetMatrix4x4(), sizeof ( float ) * 16 ) == 0;
    }
}

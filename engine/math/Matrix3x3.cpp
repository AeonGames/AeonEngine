/*
Copyright (C) 2014-2019 Rodrigo Jose Hernandez Cordoba

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
#include <cmath>
#include "aeongames/Matrix3x3.h"
#include "aeongames/Transform.h"
#include "aeongames/Vector3.h"
#include "3DMath.h"

namespace AeonGames
{
    static_assert ( sizeof ( Matrix3x3 ) == sizeof ( float ) * 9, "Size of Matrix3x3 is not the same as float[9]." );
    Matrix3x3::Matrix3x3() : mMatrix{1, 0, 0, 0, 1, 0, 0, 0, 1} {}

    Matrix3x3::Matrix3x3 ( const Transform& aTransform ) : Matrix3x3{ aTransform.GetScaleRotationMatrix() }
    {
    }

    Matrix3x3::Matrix3x3 ( const float* const aMatrix3x3 )
    {
        memcpy ( mMatrix, aMatrix3x3, sizeof ( float ) * 9 );
    }

    Matrix3x3::Matrix3x3 ( const std::initializer_list<const float> aList )
    {
        const float* scalar = aList.begin();
        for ( size_t i = 0; i < 9; ++i )
        {
            mMatrix[i] = ( scalar != aList.end() ) ? *scalar++ : ( i % 4 ) ? 0.0f : 1.0f;
        }
    }

    const float* const Matrix3x3::GetMatrix3x3() const
    {
        return mMatrix;
    }

    Matrix3x3& Matrix3x3::Invert()
    {
        Invert3x3Matrix ( mMatrix, mMatrix );
        return *this;
    }

    const Matrix3x3 Matrix3x3::GetInvertedMatrix3x3()
    {
        return Matrix3x3 ( *this ).Invert();
    }

    Matrix3x3& Matrix3x3::Rotate ( float angle, float x, float y, float z )
    {
        /*
        This replicates how glRotatef works.
        */
        return *this *= GetRotationMatrix ( angle, x, y, z );
    }

    Matrix3x3& Matrix3x3::operator *= ( const Matrix3x3& aRight )
    {
        // Item 22 from MEC++
        float local[9]
        {
            mMatrix[0], mMatrix[1], mMatrix[2],
            mMatrix[3], mMatrix[4], mMatrix[5],
            mMatrix[6], mMatrix[7], mMatrix[8]
        };

        // Column mayor (OpenGL way)
        mMatrix[0] = local[0] * aRight.mMatrix[0] + local[3] * aRight.mMatrix[1] + local[6] * aRight.mMatrix[2];
        mMatrix[1] = local[1] * aRight.mMatrix[0] + local[4] * aRight.mMatrix[1] + local[7] * aRight.mMatrix[2];
        mMatrix[2] = local[2] * aRight.mMatrix[0] + local[5] * aRight.mMatrix[1] + local[8] * aRight.mMatrix[2];

        mMatrix[3] = local[0] * aRight.mMatrix[3] + local[3] * aRight.mMatrix[4] + local[6] * aRight.mMatrix[5];
        mMatrix[4] = local[1] * aRight.mMatrix[3] + local[4] * aRight.mMatrix[4] + local[7] * aRight.mMatrix[5];
        mMatrix[5] = local[2] * aRight.mMatrix[3] + local[5] * aRight.mMatrix[4] + local[8] * aRight.mMatrix[5];

        mMatrix[6] = local[0] * aRight.mMatrix[6] + local[3] * aRight.mMatrix[7] + local[6] * aRight.mMatrix[8];
        mMatrix[7] = local[1] * aRight.mMatrix[6] + local[4] * aRight.mMatrix[7] + local[7] * aRight.mMatrix[8];
        mMatrix[8] = local[2] * aRight.mMatrix[6] + local[5] * aRight.mMatrix[7] + local[8] * aRight.mMatrix[8];

        return *this;
    }

    const float Matrix3x3::operator[] ( size_t aIndex ) const
    {
        assert ( aIndex < 9 );
        return mMatrix[aIndex];
    }

    const Matrix3x3 Matrix3x3::GetRotationMatrix ( float angle, float x, float y, float z )
    {
        auto radians = float ( ( angle / 180.0f ) * static_cast<float> ( M_PI ) );
        float c = cosf ( radians );
        float s = sinf ( radians );
        return Matrix3x3
        {
            x * x * ( 1 - c ) + c,
            x * y * ( 1 - c ) - z * s,
            x * z * ( 1 - c ) + y * s,
            y * x * ( 1 - c ) + z * s,
            y * y * ( 1 - c ) + c,
            y * z * ( 1 - c ) - x * s,
            x * z * ( 1 - c ) - y * s,
            y * z * ( 1 - c ) + x * s,
            z * z * ( 1 - c ) + c,
        };
    }

    const Matrix3x3 operator* ( const Matrix3x3& lhs, const Matrix3x3& rhs )
    {
        /*  Here Matrix3x3(lhs) *MAY* mean cast away constness
            rather tan create a temporary object in some compilers,
            we want the temporary, NOT the cast.*/
        return Matrix3x3 ( lhs ) *= rhs;
    }

    const Vector3 operator* ( const Matrix3x3 & lhs, const Vector3 & rhs )
    {
        return Vector3
        {
            rhs[0] * lhs[0] + rhs[1] * lhs[3] + rhs[2] * lhs[6],
            rhs[0] * lhs[1] + rhs[1] * lhs[4] + rhs[2] * lhs[7],
            rhs[0] * lhs[2] + rhs[1] * lhs[5] + rhs[2] * lhs[8]
        };
    }

    const bool operator== ( const Matrix3x3& lhs, const Matrix3x3& rhs )
    {
        return memcmp ( lhs.GetMatrix3x3(), rhs.GetMatrix3x3(), sizeof ( float ) * 9 ) == 0;
    }

    const Matrix3x3 Abs ( const Matrix3x3& aMatrix3x3 )
    {
        return Matrix3x3
        {
            std::abs ( aMatrix3x3[0] ),
            std::abs ( aMatrix3x3[1] ),
            std::abs ( aMatrix3x3[2] ),
            std::abs ( aMatrix3x3[3] ),
            std::abs ( aMatrix3x3[4] ),
            std::abs ( aMatrix3x3[5] ),
            std::abs ( aMatrix3x3[6] ),
            std::abs ( aMatrix3x3[7] ),
            std::abs ( aMatrix3x3[8] )
        };
    }
}

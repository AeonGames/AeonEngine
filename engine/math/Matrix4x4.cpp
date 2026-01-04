/*
Copyright (C) 2014-2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <ostream>
#include <iomanip>
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Vector3.hpp"
#include "3DMath.h"

namespace AeonGames
{
    static_assert ( sizeof ( Matrix4x4 ) == sizeof ( float ) * 16, "Size of Matrix4x4 is not the same as float[16]." );
    Matrix4x4::Matrix4x4()
    {
        SetIdentityMatrix4x4 ( mMatrix );
    }

    Matrix4x4::Matrix4x4 ( const Transform& aTransform ) : Matrix4x4{ aTransform.GetMatrix() }
    {
    }

    Matrix4x4::Matrix4x4 ( const float* const aMatrix4x4 )
    {
        memcpy ( mMatrix, aMatrix4x4, sizeof ( float ) * 16 );
    }

    Matrix4x4::Matrix4x4 ( const std::initializer_list<const float> aList )
    {
        const float* scalar = aList.begin();
        for ( size_t i = 0; i < 16; ++i )
        {
            mMatrix[i] = ( scalar != aList.end() ) ? *scalar++ : ( i % 5 ) ? 0.0f : 1.0f;
        }
    }

    Matrix4x4::~Matrix4x4()
        = default;

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
        // glFrustum, +X right, +Y forward, +Z up
        // X
        mMatrix[0] = ( 2 * aNear ) / ( aRight - aLeft );
        mMatrix[1] = 0.0f;
        mMatrix[2] = 0.0f;
        mMatrix[3] = 0.0f;

        // Y
        mMatrix[4] = - ( ( aRight + aLeft ) / ( aRight - aLeft ) );
        mMatrix[5] = - ( ( aBottom + aTop ) / ( aBottom - aTop ) );
        mMatrix[6] = ( ( aFar + aNear ) / ( aFar - aNear ) );
        mMatrix[7] = 1.0f;

        // Z
        mMatrix[8] = 0.0f;
        mMatrix[9] = ( 2 * aNear ) / ( aBottom - aTop );
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
        float fH = std::tan ( aFieldOfVision * static_cast<float> ( M_PI ) / 360.0f ) * aNear;
        float fW = fH * aAspect;
        Frustum ( -fW, fW, -fH, fH, aNear, aFar );
    }

    Matrix4x4 & Matrix4x4::Rotate ( float angle, float x, float y, float z )
    {
        /*
        This replicates how glRotatef works.
        */
        return *this *= GetRotationMatrix ( angle, x, y, z );
    }

    Matrix4x4& Matrix4x4::operator *= ( const Matrix4x4& lhs )
    {
        // Item 22 from MEC++
        float local[16]
        {
            mMatrix[0], mMatrix[1], mMatrix[2], mMatrix[3],
            mMatrix[4], mMatrix[5], mMatrix[6], mMatrix[7],
            mMatrix[8], mMatrix[9], mMatrix[10], mMatrix[11],
            mMatrix[12], mMatrix[13], mMatrix[14], mMatrix[15]
        };
        mMatrix[0] = local[0] * lhs.mMatrix[0] + local[4] * lhs.mMatrix[1] + local[8] * lhs.mMatrix[2] + local[12] * lhs.mMatrix[3];
        mMatrix[1] = local[1] * lhs.mMatrix[0] + local[5] * lhs.mMatrix[1] + local[9] * lhs.mMatrix[2] + local[13] * lhs.mMatrix[3];
        mMatrix[2] = local[2] * lhs.mMatrix[0] + local[6] * lhs.mMatrix[1] + local[10] * lhs.mMatrix[2] + local[14] * lhs.mMatrix[3];
        mMatrix[3] = local[3] * lhs.mMatrix[0] + local[7] * lhs.mMatrix[1] + local[11] * lhs.mMatrix[2] + local[15] * lhs.mMatrix[3];

        mMatrix[4] = local[0] * lhs.mMatrix[4] + local[4] * lhs.mMatrix[5] + local[8] * lhs.mMatrix[6] + local[12] * lhs.mMatrix[7];
        mMatrix[5] = local[1] * lhs.mMatrix[4] + local[5] * lhs.mMatrix[5] + local[9] * lhs.mMatrix[6] + local[13] * lhs.mMatrix[7];
        mMatrix[6] = local[2] * lhs.mMatrix[4] + local[6] * lhs.mMatrix[5] + local[10] * lhs.mMatrix[6] + local[14] * lhs.mMatrix[7];
        mMatrix[7] = local[3] * lhs.mMatrix[4] + local[7] * lhs.mMatrix[5] + local[11] * lhs.mMatrix[6] + local[15] * lhs.mMatrix[7];

        mMatrix[8] = local[0] * lhs.mMatrix[8] + local[4] * lhs.mMatrix[9] + local[8] * lhs.mMatrix[10] + local[12] * lhs.mMatrix[11];
        mMatrix[9] = local[1] * lhs.mMatrix[8] + local[5] * lhs.mMatrix[9] + local[9] * lhs.mMatrix[10] + local[13] * lhs.mMatrix[11];
        mMatrix[10] = local[2] * lhs.mMatrix[8] + local[6] * lhs.mMatrix[9] + local[10] * lhs.mMatrix[10] + local[14] * lhs.mMatrix[11];
        mMatrix[11] = local[3] * lhs.mMatrix[8] + local[7] * lhs.mMatrix[9] + local[11] * lhs.mMatrix[10] + local[15] * lhs.mMatrix[11];

        mMatrix[12] = local[0] * lhs.mMatrix[12] + local[4] * lhs.mMatrix[13] + local[8] * lhs.mMatrix[14] + local[12] * lhs.mMatrix[15];
        mMatrix[13] = local[1] * lhs.mMatrix[12] + local[5] * lhs.mMatrix[13] + local[9] * lhs.mMatrix[14] + local[13] * lhs.mMatrix[15];
        mMatrix[14] = local[2] * lhs.mMatrix[12] + local[6] * lhs.mMatrix[13] + local[10] * lhs.mMatrix[14] + local[14] * lhs.mMatrix[15];
        mMatrix[15] = local[3] * lhs.mMatrix[12] + local[7] * lhs.mMatrix[13] + local[11] * lhs.mMatrix[14] + local[15] * lhs.mMatrix[15];
        return *this;
    }

    const float Matrix4x4::operator[] ( size_t aIndex ) const
    {
        assert ( aIndex < 16 );
        return mMatrix[aIndex];
    }

    const Matrix4x4 Matrix4x4::GetRotationMatrix ( float angle, float x, float y, float z )
    {
        auto radians = float ( ( angle / 180.0f ) * M_PI );
        float c = cosf ( radians );
        float s = sinf ( radians );
        return Matrix4x4
        {
            x * x * ( 1 - c ) + c,
            x * y * ( 1 - c ) - z * s,
            x * z * ( 1 - c ) + y * s,
            0,
            y * x * ( 1 - c ) + z * s,
            y * y * ( 1 - c ) + c,
            y * z * ( 1 - c ) - x * s,
            0,
            x * z * ( 1 - c ) - y * s,
            y * z * ( 1 - c ) + x * s,
            z * z * ( 1 - c ) + c,
            0,
            0, 0, 0, 1
        };
    }

    const Matrix4x4 operator* ( const Matrix4x4& lhs, const Matrix4x4& rhs )
    {
        /*  Here Matrix4x4(lhs) *MAY* mean cast away constness
            rather tan create a temporary object in some compilers,
            we want the temporary, NOT the cast.*/
        return Matrix4x4 ( lhs ) *= rhs;
    }

    const Vector3 operator* ( const Matrix4x4 & lhs, const Vector3 & rhs )
    {
        return Vector3
        {
            rhs[0] * lhs[0] + rhs[1] * lhs[4] + rhs[2] * lhs[8] + lhs[12],
            rhs[0] * lhs[1] + rhs[1] * lhs[5] + rhs[2] * lhs[9] + lhs[13],
            rhs[0] * lhs[2] + rhs[1] * lhs[6] + rhs[2] * lhs[10] + lhs[14]
        };
    }

    const bool operator== ( const Matrix4x4& lhs, const Matrix4x4& rhs )
    {
        return memcmp ( lhs.GetMatrix4x4(), rhs.GetMatrix4x4(), sizeof ( float ) * 16 ) == 0;
    }

    const Matrix4x4 Abs ( const Matrix4x4& aMatrix4x4 )
    {
        return Matrix4x4
        {
            std::abs ( aMatrix4x4[0] ),
            std::abs ( aMatrix4x4[1] ),
            std::abs ( aMatrix4x4[2] ),
            std::abs ( aMatrix4x4[3] ),
            std::abs ( aMatrix4x4[4] ),
            std::abs ( aMatrix4x4[5] ),
            std::abs ( aMatrix4x4[6] ),
            std::abs ( aMatrix4x4[7] ),
            std::abs ( aMatrix4x4[8] ),
            std::abs ( aMatrix4x4[9] ),
            std::abs ( aMatrix4x4[10] ),
            std::abs ( aMatrix4x4[11] ),
            std::abs ( aMatrix4x4[12] ),
            std::abs ( aMatrix4x4[13] ),
            std::abs ( aMatrix4x4[14] ),
            std::abs ( aMatrix4x4[15] )
        };
    }

    std::ostream& operator<< ( std::ostream& os, const Matrix4x4& aMatrix )
    {
        const float* m = aMatrix.GetMatrix4x4();
        os << std::fixed << std::setprecision ( 4 );
        os << "[\n";
        for ( int row = 0; row < 4; ++row )
        {
            os << "  [ ";
            for ( int col = 0; col < 4; ++col )
            {
                os << std::setw ( 10 ) << m[col * 4 + row];
                if ( col < 3 ) os << ", ";
            }
            os << " ]\n";
        }
        os << "]";
        return os;
    }

    const Matrix4x4 Matrix4x4::Identity{};
}

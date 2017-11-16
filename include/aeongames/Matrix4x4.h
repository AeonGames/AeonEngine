/*
Copyright (C) 2015-2017 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MATRIX4X4_H
#define AEONGAMES_MATRIX4X4_H
/*! \file
    \brief Header for 4x4 matrix class.
    \author Rodrigo Hernandez.
    \copy 2015-2016
*/

#include "aeongames/Platform.h"

namespace AeonGames
{
    class Vector3;
    class Transform;
    /*! \brief 4 by 4 matrix in colum mayor order.
        \ingroup placeables
    */
    class Matrix4x4
    {
    public:
        ///@brief Default constructor.
        DLL Matrix4x4();
        /*! @brief Constructor
            @param aTransform Transform from which to initialize the matrix.
        */
        DLL Matrix4x4 ( const Transform& aTransform );
        /*! @brief Constructor
        @param aSRT a float pointer or array containing scale rotation and translation information in that order
        consisting of 3 values XYZ for scale, 4 values WXYZ quaternion for rotation and 3 values XYZ for translation.
        @note This is the same format as the values are internally stored and what Transform::GetTransform returns.
        */
        DLL Matrix4x4 ( const float* const aMatrix );
        DLL Matrix4x4 (
            const float m00, const float m01, const float m02, const float m03,
            const float m10, const float m11, const float m12, const float m13,
            const float m20, const float m21, const float m22, const float m23,
            const float m30, const float m31, const float m32, const float m33 );
        /// destructor.
        DLL ~Matrix4x4();
        DLL const float* const GetMatrix4x4() const;
        DLL const Matrix4x4 GetInvertedMatrix4x4();
        DLL Matrix4x4& Invert();

        DLL void Frustum ( float aLeft, float aRight, float aBottom, float aTop, float aNear, float aFar );
        DLL void Ortho ( float aLeft, float aRight, float aBottom, float aTop, float aNear, float aFar );
        DLL void Perspective ( float aFieldOfVision, float aAspect, float aNear, float aFar );

        DLL Matrix4x4& Rotate ( float angle, float x, float y, float z );
        /*! \name Operators */
        //@{
        DLL Matrix4x4& operator*= ( const Matrix4x4& lhs );
        DLL const float operator[] ( size_t aIndex ) const;
        //@}
        /*! \brief Constructs the rotation matrix defined by the axis-angle provided.

        The Matrix returned is a 4x4 matrix constructed using the same formula glRotate* uses.

        \param angle [in] Angle in degrees of rotation.
        \param x [in] X element of axis of rotation.
        \param y [in] Y element of axis of rotation.
        \param z [in] Z element of axis of rotation.
        \return Rotation matrix.
        */
        DLL static const Matrix4x4 GetRotationMatrix ( float angle, float x, float y, float z );
    protected:
        /// Lineal row mayor matrix.
        float mMatrix[16];
    };
    /*! \brief Multiplies two 4x4 matrices.
    Multiplies two 4x4 matrices
    \param lhs [in] left side matrix.
    \param rhs [in] right side matrix.
    \return The resulting matrix.
    \note Since the matrices are stored in column mayor order, this is post multiplication.
    */
    DLL const Matrix4x4 operator* ( const Matrix4x4& lhs, const Matrix4x4& rhs );
    DLL const Vector3 operator* ( const Matrix4x4& lhs, const Vector3& rhs );
    DLL const bool operator== ( const Matrix4x4& lhs, const Matrix4x4& rhs );
    static_assert ( sizeof ( Matrix4x4 ) == ( sizeof ( float ) * 16 ), "Matrix4x4 is not 16 floats wide." );
}
#endif

/*
Copyright (C) 2015-2019,2025 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MATRIX3X3_H
#define AEONGAMES_MATRIX3X3_H
/*! \file
    \brief Header for 3x3 matrix class.
    \author Rodrigo Hernandez.
    \copy 2015-2019
*/

#include "aeongames/Platform.hpp"
#include <initializer_list>

namespace AeonGames
{
    class Vector3;
    class Transform;
    /*! \brief 3 by 3 matrix in colum mayor order. */
    class Matrix3x3
    {
    public:
        ///@brief Default constructor.
        DLL Matrix3x3();
        /*! @brief Constructor
            @param aTransform Transform from which to initialize the matrix. Only Scale and Rotation are used.
        */
        DLL Matrix3x3 ( const Transform& aTransform );
        DLL Matrix3x3 ( const float* const aMatrix );
        DLL Matrix3x3 ( const std::initializer_list<const float> aList );
        DLL const float* const GetMatrix3x3() const;
        DLL const Matrix3x3 GetInvertedMatrix3x3();
        DLL Matrix3x3& Invert();
        DLL Matrix3x3& Rotate ( float angle, float x, float y, float z );
        /*! \name Operators */
        //@{
        DLL Matrix3x3& operator*= ( const Matrix3x3& lhs );
        DLL const float operator[] ( size_t aIndex ) const;
        //@}
        /*! \brief Constructs the rotation matrix defined by the axis-angle provided.

        The Matrix returned is a 3x3 matrix constructed using the same formula glRotate* uses.

        \param angle [in] Angle in degrees of rotation.
        \param x [in] X element of axis of rotation.
        \param y [in] Y element of axis of rotation.
        \param z [in] Z element of axis of rotation.
        \return Rotation matrix.
        */
        DLL static const Matrix3x3 GetRotationMatrix ( float angle, float x, float y, float z );
    protected:
        /// Lineal row mayor matrix.
        float mMatrix[9];
    };
    /*! \brief Multiplies two 3x3 matrices.
    Multiplies two 3x3 matrices
    \param lhs [in] left side matrix.
    \param rhs [in] right side matrix.
    \return The resulting matrix.
    \note Since the matrices are stored in column mayor order, this is post multiplication.
    */
    DLL const Matrix3x3 operator* ( const Matrix3x3& lhs, const Matrix3x3& rhs );
    DLL const Vector3 operator* ( const Matrix3x3& lhs, const Vector3& rhs );
    DLL const bool operator== ( const Matrix3x3& lhs, const Matrix3x3& rhs );
    DLL const Matrix3x3 Abs ( const Matrix3x3& aMatrix3x3 );
    static_assert ( sizeof ( Matrix3x3 ) == ( sizeof ( float ) * 9 ), "Matrix3x3 is not 9 floats wide." );
}
#endif

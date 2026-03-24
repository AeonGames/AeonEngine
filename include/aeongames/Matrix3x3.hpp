/*
Copyright (C) 2015-2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
    \copyright 2015-2019
*/

#include "aeongames/Platform.hpp"
#include <initializer_list>

namespace AeonGames
{
    class Vector3;
    class Transform;
    /*! \brief 3 by 3 matrix in column major order. */
    class Matrix3x3
    {
    public:
        ///@brief Default constructor.
        DLL Matrix3x3();
        /*! @brief Constructor
            @param aTransform Transform from which to initialize the matrix. Only Scale and Rotation are used.
        */
        DLL Matrix3x3 ( const Transform& aTransform );
        /** @brief Construct from a float array.
            @param aMatrix Pointer to a float array of at least 9 elements in column-major order.
        */
        DLL Matrix3x3 ( const float* const aMatrix );
        /** @brief Construct from an initializer list.
            @param aList Initializer list of float values for the matrix elements.
        */
        DLL Matrix3x3 ( const std::initializer_list<const float> aList );
        /** @brief Get a pointer to the internal matrix data.
            @return Pointer to the 9 float elements of the matrix.
        */
        DLL const float* const GetMatrix3x3() const;
        /** @brief Get the inverted matrix.
            @return A new Matrix3x3 that is the inverse of this matrix.
        */
        DLL const Matrix3x3 GetInvertedMatrix3x3();
        /** @brief Invert this matrix in place.
            @return Reference to this matrix after inversion.
        */
        DLL Matrix3x3& Invert();
        /** @brief Apply a rotation to this matrix.
            @param angle Angle in degrees of rotation.
            @param x X component of the axis of rotation.
            @param y Y component of the axis of rotation.
            @param z Z component of the axis of rotation.
            @return Reference to this matrix after rotation.
        */
        DLL Matrix3x3& Rotate ( float angle, float x, float y, float z );
        /*! \name Operators */
        //@{
        /// @brief Multiply this matrix by another matrix.
        DLL Matrix3x3& operator*= ( const Matrix3x3& lhs );
        /// @brief Access a matrix element by index.
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
        /// Linear row major matrix.
        float mMatrix[9];
    };
    /*! \brief Multiplies two 3x3 matrices.
    Multiplies two 3x3 matrices
    \param lhs [in] left side matrix.
    \param rhs [in] right side matrix.
    \return The resulting matrix.
    \note Since the matrices are stored in column major order, this is post multiplication.
    */
    DLL const Matrix3x3 operator* ( const Matrix3x3& lhs, const Matrix3x3& rhs );
    /** @brief Multiply a 3x3 matrix by a 3D vector.
        @param lhs The matrix.
        @param rhs The vector.
        @return The transformed vector.
    */
    DLL const Vector3 operator* ( const Matrix3x3& lhs, const Vector3& rhs );
    /** @brief Compare two 3x3 matrices for equality.
        @param lhs Left-hand side matrix.
        @param rhs Right-hand side matrix.
        @return True if the matrices are equal.
    */
    DLL const bool operator== ( const Matrix3x3& lhs, const Matrix3x3& rhs );
    /** @brief Compute the element-wise absolute value of a matrix.
        @param aMatrix3x3 The input matrix.
        @return A new matrix with absolute values of each element.
    */
    DLL const Matrix3x3 Abs ( const Matrix3x3& aMatrix3x3 );
    static_assert ( sizeof ( Matrix3x3 ) == ( sizeof ( float ) * 9 ), "Matrix3x3 is not 9 floats wide." );
}
#endif

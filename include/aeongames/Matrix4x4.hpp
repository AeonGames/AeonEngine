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
#ifndef AEONGAMES_MATRIX4X4_H
#define AEONGAMES_MATRIX4X4_H
/*! \file
    \brief Header for 4x4 matrix class.
    \author Rodrigo Hernandez.
    \copyright 2015-2018
*/

#include "aeongames/Platform.hpp"
#include <initializer_list>
#include <iosfwd>

namespace AeonGames
{
    class Vector3;
    class Transform;
    /*! \brief 4 by 4 matrix in colum mayor order.
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
        /** @brief Construct from a float array.
            @param aMatrix Pointer to a float array of at least 16 elements in column-major order.
        */
        DLL Matrix4x4 ( const float* const aMatrix );
        /** @brief Construct from an initializer list.
            @param aList Initializer list of float values for the matrix elements.
        */
        DLL Matrix4x4 ( const std::initializer_list<const float> aList );
        /// destructor.
        DLL ~Matrix4x4();
        /** @brief Get a pointer to the internal matrix data.
            @return Pointer to the 16 float elements of the matrix.
        */
        DLL const float* const GetMatrix4x4() const;
        /** @brief Get the inverted matrix.
            @return A new Matrix4x4 that is the inverse of this matrix.
        */
        DLL const Matrix4x4 GetInvertedMatrix4x4();
        /** @brief Invert this matrix in place.
            @return Reference to this matrix after inversion.
        */
        DLL Matrix4x4& Invert();

        /** @brief Set up a perspective projection matrix defined by a frustum.
            @param aLeft Left vertical clipping plane.
            @param aRight Right vertical clipping plane.
            @param aBottom Bottom horizontal clipping plane.
            @param aTop Top horizontal clipping plane.
            @param aNear Near depth clipping plane distance.
            @param aFar Far depth clipping plane distance.
        */
        DLL void Frustum ( float aLeft, float aRight, float aBottom, float aTop, float aNear, float aFar );
        /** @brief Set up an orthographic projection matrix.
            @param aLeft Left vertical clipping plane.
            @param aRight Right vertical clipping plane.
            @param aBottom Bottom horizontal clipping plane.
            @param aTop Top horizontal clipping plane.
            @param aNear Near depth clipping plane distance.
            @param aFar Far depth clipping plane distance.
        */
        DLL void Ortho ( float aLeft, float aRight, float aBottom, float aTop, float aNear, float aFar );
        /** @brief Set up a symmetric perspective projection matrix.
            @param aFieldOfVision Vertical field of vision angle in degrees.
            @param aAspect Aspect ratio (width / height).
            @param aNear Near depth clipping plane distance.
            @param aFar Far depth clipping plane distance.
        */
        DLL void Perspective ( float aFieldOfVision, float aAspect, float aNear, float aFar );

        /** @brief Apply a rotation to this matrix.
            @param angle Angle in degrees of rotation.
            @param x X component of the axis of rotation.
            @param y Y component of the axis of rotation.
            @param z Z component of the axis of rotation.
            @return Reference to this matrix after rotation.
        */
        DLL Matrix4x4& Rotate ( float angle, float x, float y, float z );
        /*! \name Operators */
        //@{
        /// @brief Multiply this matrix by another matrix.
        DLL Matrix4x4& operator*= ( const Matrix4x4& lhs );
        /// @brief Access a matrix element by index.
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
        /** @brief The 4x4 identity matrix constant. */
        DLL static const Matrix4x4 Identity;
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
    /** @brief Multiply a 4x4 matrix by a 3D vector.
        @param lhs The matrix.
        @param rhs The vector.
        @return The transformed vector.
    */
    DLL const Vector3 operator* ( const Matrix4x4& lhs, const Vector3& rhs );
    /** @brief Compare two 4x4 matrices for equality.
        @param lhs Left-hand side matrix.
        @param rhs Right-hand side matrix.
        @return True if the matrices are equal.
    */
    DLL const bool operator== ( const Matrix4x4& lhs, const Matrix4x4& rhs );
    /** @brief Compute the element-wise absolute value of a matrix.
        @param aMatrix4x4 The input matrix.
        @return A new matrix with absolute values of each element.
    */
    DLL const Matrix4x4 Abs ( const Matrix4x4& aMatrix4x4 );
    /** @brief Stream output operator for a 4x4 matrix.
        @param os The output stream.
        @param aMatrix The matrix to output.
        @return Reference to the output stream.
    */
    DLL std::ostream& operator<< ( std::ostream& os, const Matrix4x4& aMatrix );
    static_assert ( sizeof ( Matrix4x4 ) == ( sizeof ( float ) * 16 ), "Matrix4x4 is not 16 floats wide." );
}
#endif

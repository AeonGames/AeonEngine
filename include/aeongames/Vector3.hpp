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
#ifndef AEONGAMES_VECTOR3_H
#define AEONGAMES_VECTOR3_H
/*! \file
    \brief Header for the 3D vector class.
    \author Rodrigo Hernandez.
    \copyright 2015-2017
*/

#include "aeongames/Platform.hpp"
#include <cstdint>

namespace AeonGames
{
    class Plane;
    /*! \brief 3D vector class. */
    class Vector3
    {
    public:
        ///@brief Default constructor.
        DLL Vector3();
        /*! @brief Constructor
        @param aVector a float pointer or array containing vector data.
        @note This is the same format the values are internally stored and what Vector3::GetVector3 returns.
        */
        DLL Vector3 ( const float* const aVector );
        /** @brief Constructor from raw memory with stride.
            @param aVector Pointer to raw memory containing vector data.
            @param aStride Byte stride between consecutive float components.
        */
        DLL Vector3 ( const void* const aVector, const uint32_t aStride );
        /** @brief Constructor.
            @param aX X component.
            @param aY Y component.
            @param aZ Z component.
        */
        DLL Vector3 ( float aX, float aY, float aZ );
        /** @brief Copy vector data to an external float array.
            @param[out] aData Pointer to a float array of at least 3 elements.
        */
        DLL void Get ( float* aData ) const;
        /** @brief Get a pointer to the internal vector data.
            @return Pointer to the float array containing X,Y,Z components.
        */
        DLL const float* const GetVector3() const;
        /** @brief Get the X component.
            @return Reference to the X component.
        */
        DLL const float& GetX() const;
        /** @brief Get the Y component.
            @return Reference to the Y component.
        */
        DLL const float& GetY() const;
        /** @brief Get the Z component.
            @return Reference to the Z component.
        */
        DLL const float& GetZ() const;
        /** @brief Set the vector from a float array.
            @param aVector Pointer to a float array containing X,Y,Z values.
        */
        DLL void SetVector3 ( const float* const aVector );
        /** @brief Set the vector from raw memory with stride.
            @param aVector Pointer to raw memory containing vector data.
            @param aStride Byte stride between consecutive float components.
        */
        DLL void SetVector3 ( const void* const aVector, const uint32_t aStride );
        /** @brief Get the squared length of the vector.
            @return Squared magnitude of the vector.
        */
        DLL float GetLengthSquared() const;
        /** @brief Get the length of the vector.
            @return Magnitude of the vector.
        */
        DLL float GetLength() const;
        /** @brief Get the index of the axis with the largest absolute value.
            @return Index (0=X, 1=Y, 2=Z) of the largest axis.
        */
        DLL size_t GetMaxAxisIndex() const;
        /** @brief Get the index of the axis with the smallest absolute value.
            @return Index (0=X, 1=Y, 2=Z) of the smallest axis.
        */
        DLL size_t GetMinAxisIndex() const;
        /** @brief Get the absolute value of the largest axis component.
            @return Absolute value of the largest component.
        */
        DLL float GetMaxAxisLength() const;
        /** @brief Get the absolute value of the smallest axis component.
            @return Absolute value of the smallest component.
        */
        DLL float GetMinAxisLength() const;
        /** @brief Get the signed distance from this point to a plane.
            @param aPlane The plane to measure distance to.
            @return Signed distance to the plane.
        */
        DLL float GetDistanceToPlane ( const Plane& aPlane ) const;
        /** @brief Check whether all components are zero.
            @return true if the vector is the zero vector.
        */
        DLL bool IsZero() const;
        /*! \name Operators */
        //@{
        /// @brief Assign from a raw float array.
        DLL Vector3& operator= ( const float* aLhs );
        /// @brief Subtract a vector from this vector.
        DLL Vector3& operator-= ( const Vector3& aLhs );
        /// @brief Add a vector to this vector.
        DLL Vector3& operator+= ( const Vector3& aLhs );
        /// @brief Scale this vector by a scalar.
        DLL Vector3& operator*= ( const float aLhs );
        /// @brief Component-wise multiply by another vector.
        DLL Vector3& operator*= ( const Vector3& aLhs );
        /// @brief Divide this vector by a scalar.
        DLL Vector3& operator/= ( const float aLhs );
        /// @brief Access a component by index (const).
        DLL float operator[] ( const size_t aIndex ) const;
        /// @brief Access a component by index.
        DLL float& operator [] ( const size_t aIndex );
        //@}
    protected:
        /// X,Y,Z
        float mVector[3];
    };
    /** @brief Subtraction operator.
        @param aLhs Left-hand side vector.
        @param aRhs Right-hand side vector.
        @return Resulting vector (aLhs - aRhs).
    */
    DLL const Vector3 operator- ( const Vector3& aLhs, const Vector3& aRhs );
    /** @brief Addition operator.
        @param aLhs Left-hand side vector.
        @param aRhs Right-hand side vector.
        @return Resulting vector (aLhs + aRhs).
    */
    DLL const Vector3 operator+ ( const Vector3& aLhs, const Vector3& aRhs );
    /** @brief Scalar multiplication operator (vector * scalar).
        @param aLhs Vector operand.
        @param aRhs Scalar operand.
        @return Scaled vector.
    */
    DLL const Vector3 operator* ( const Vector3& aLhs, const float aRhs );
    /** @brief Scalar division operator.
        @param aLhs Vector operand.
        @param aRhs Scalar divisor.
        @return Resulting vector.
    */
    DLL const Vector3 operator/ ( const Vector3& aLhs, const float aRhs );
    /** @brief Scalar multiplication operator (scalar * vector).
        @param aLhs Scalar operand.
        @param aRhs Vector operand.
        @return Scaled vector.
    */
    DLL const Vector3 operator* ( const float aLhs, const Vector3& aRhs );
    /** @brief Component-wise multiplication operator.
        @param aLhs Left-hand side vector.
        @param aRhs Right-hand side vector.
        @return Component-wise product of the two vectors.
    */
    DLL const Vector3 operator* ( const Vector3& aLhs, const Vector3& aRhs );
    /** @brief Inequality comparison operator.
        @param aLhs Left-hand side vector.
        @param aRhs Right-hand side vector.
        @return true if the vectors are not equal.
    */
    DLL bool operator!= ( const Vector3& aLhs, const Vector3& aRhs );
    /** @brief Equality comparison operator.
        @param aLhs Left-hand side vector.
        @param aRhs Right-hand side vector.
        @return true if both vectors are equal.
    */
    DLL bool operator== ( const Vector3& aLhs, const Vector3& aRhs );
    /** @brief Compute the cross product of two vectors.
        @param aLhs Left-hand side vector.
        @param aRhs Right-hand side vector.
        @return Cross product vector.
    */
    DLL const Vector3 Cross ( const Vector3& aLhs, const Vector3& aRhs );
    /** @brief Compute the dot product of two vectors.
        @param aLhs Left-hand side vector.
        @param aRhs Right-hand side vector.
        @return Dot product scalar.
    */
    DLL const float Dot ( const Vector3& aLhs, const Vector3& aRhs );
    /** @brief Return a normalized (unit length) copy of the vector.
        @param aVector Vector to normalize.
        @return Normalized vector.
    */
    DLL const Vector3 Normalize ( const Vector3& aVector );
    /** @brief Return a vector with all components replaced by their absolute values.
        @param aVector Input vector.
        @return Vector with absolute component values.
    */
    DLL const Vector3 Abs ( const Vector3& aVector );
    /** @brief Catmull-Rom spline interpolation between four control points.
        @param p0 First control point.
        @param p1 Second control point (start of segment).
        @param p2 Third control point (end of segment).
        @param p3 Fourth control point.
        @param interpolation Interpolation parameter in [0,1].
        @return Interpolated vector on the spline.
    */
    DLL const Vector3 Spline ( const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, double interpolation );
}
#endif

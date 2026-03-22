/*
Copyright (C) 2017-2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_QUATERNION_H
#define AEONGAMES_QUATERNION_H
/*! \file
    \brief Header for the quaternion class.
    \author Rodrigo Hernandez.
    \copyright 2017
*/

#include "aeongames/Platform.hpp"

namespace AeonGames
{
    class Matrix4x4;
    class Matrix3x3;
    class Vector3;
    /*! \brief Quaternion class. */
    class Quaternion
    {
    public:
        ///@brief Default constructor.
        DLL Quaternion();
        /** @brief Construct a quaternion from individual components.
            @param w The scalar (real) component.
            @param x The X imaginary component.
            @param y The Y imaginary component.
            @param z The Z imaginary component.
        */
        DLL Quaternion ( float w, float x, float y, float z );
        /** @brief Construct a quaternion from a float array.
            @param aData Pointer to an array of at least 4 floats in W, X, Y, Z order.
        */
        DLL Quaternion ( const float* aData );
        /** @brief Copy the quaternion components to a float array.
            @param aData Pointer to an array of at least 4 floats to receive W, X, Y, Z values.
        */
        DLL void Get ( float* aData ) const;
        /// destructor.
        DLL ~Quaternion();
        /** @brief Convert the quaternion to a 4x4 rotation matrix.
            @return The equivalent 4x4 rotation matrix.
        */
        DLL Matrix4x4 GetMatrix4x4() const;
        /** @brief Convert the quaternion to a 3x3 rotation matrix.
            @return The equivalent 3x3 rotation matrix.
        */
        DLL Matrix3x3 GetMatrix3x3() const;
        /*! \name Operators */
        //@{
        /// @brief Assign from a raw float array.
        DLL Quaternion& operator= ( const float* aLhs );
        /// @brief Multiply this quaternion by another quaternion.
        DLL Quaternion& operator*= ( const Quaternion& lhs );
        /// @brief Access a quaternion component by index (const).
        DLL float operator[] ( const size_t aIndex ) const;
        /// @brief Access a quaternion component by index.
        DLL float& operator [] ( const size_t aIndex );
        //@}
        /** @brief Create a quaternion from an axis-angle representation.
            @param angle Angle in degrees of rotation.
            @param x X component of the axis of rotation.
            @param y Y component of the axis of rotation.
            @param z Z component of the axis of rotation.
            @return The resulting quaternion.
        */
        DLL static const Quaternion GetFromAxisAngle ( float angle, float x, float y, float z );
        /** @brief Create a quaternion from Euler angles.
            @param aEuler A vector containing Euler angles in degrees.
            @return The resulting quaternion.
        */
        DLL static const Quaternion GetFromEuler ( const Vector3& aEuler );
        /** @brief Normalize this quaternion in place.
            @return Reference to this quaternion after normalization.
        */
        DLL Quaternion& Normalize();
        /** @brief Get the Euler angle representation of this quaternion.
            @return A vector containing the Euler angles in degrees.
        */
        DLL Vector3 GetEuler() const;
        /** @brief Set the quaternion from Euler angles.
            @param aEuler A vector containing the Euler angles in degrees.
            @return Reference to this quaternion.
        */
        DLL Quaternion& SetEuler ( const Vector3& aEuler );
    private:
        /// W,X,Y,Z
        float mQuaternion[4] {1.0f, 0.0f, 0.0f, 0.0f};
    };
    /** @brief Multiply two quaternions.
        @param lhs Left-hand side quaternion.
        @param rhs Right-hand side quaternion.
        @return The product quaternion.
    */
    DLL const Quaternion operator* ( const Quaternion& lhs, const Quaternion& rhs );
    /** @brief Rotate a 3D vector by a quaternion.
        @param lhs The quaternion representing the rotation.
        @param rhs The vector to rotate.
        @return The rotated vector.
    */
    DLL const Vector3 operator* ( const Quaternion& lhs, const Vector3& rhs );
    /** @brief Compare two quaternions for equality.
        @param lhs Left-hand side quaternion.
        @param rhs Right-hand side quaternion.
        @return True if the quaternions are equal.
    */
    DLL bool operator== ( const Quaternion& lhs, const Quaternion& rhs );


    /*! \brief Linearly interpolate between two quaternions.

    Each element is interpolated as v' = v1+((v2-v1)*interpolation).
    The out parameter may be the same as either q1 or q2 in which case the values are overwritten.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interpolation [in] Interpolation factor.
    */
    DLL const Quaternion LerpQuats ( const Quaternion& q1, const Quaternion& q2, double interpolation );

    /*! \brief Linearly interpolate between two quaternions return the normalized result.

    Each element is interpolated as v' = v1+((v2-v1)*interpolation).
    The out parameter may be the same as either q1 or q2 in which case the values are overwritten.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interpolation [in] Interpolation factor.
    */
    DLL const Quaternion NlerpQuats ( const Quaternion& q1, const Quaternion& q2, double interpolation );

    /*! \brief Spherical Linear interpolation between two quaternions.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interpolation [in] Interpolation factor.
    */
    DLL const Quaternion SlerpQuats ( const Quaternion& q1, const Quaternion& q2, float interpolation );
}
#endif

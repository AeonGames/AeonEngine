/*
Copyright (C) 2013-2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_TRANSFORM_H
#define AEONGAMES_TRANSFORM_H
/*! \file
    \brief Header for Transform component class.
    \author Rodrigo Hernandez.
    \copyright 2013-2016
*/

#include "aeongames/Platform.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Quaternion.hpp"

namespace AeonGames
{
    class AABB;
    /*! \brief Component class for any object that requires space transformations.
    */
    class Transform
    {
    public:
        DLL Transform();
        /** @brief Construct from scale, rotation quaternion, and translation.
            @param aScale The scale vector.
            @param aRotation The rotation quaternion.
            @param aTranslation The translation vector.
        */
        DLL Transform ( const Vector3& aScale, const Quaternion& aRotation, const Vector3& aTranslation );
        /** @brief Construct from scale, rotation Euler angles, and translation.
            @param aScale The scale vector.
            @param aRotation The rotation as Euler angles in degrees.
            @param aTranslation The translation vector.
        */
        DLL Transform ( const Vector3& aScale, const Vector3& aRotation, const Vector3& aTranslation );
        /** @brief Construct from a float array.
            @param aData Pointer to a float array containing scale, rotation, and translation data.
        */
        DLL Transform ( const float* aData );
        /** @brief Copy the transform data to a float array.
            @param aData Pointer to a float array to receive the transform data.
        */
        DLL void Get ( float* aData ) const;

        /** @brief Get the scale vector.
            @return Const reference to the scale vector.
        */
        DLL const Vector3& GetScale() const;
        /** @brief Get the rotation quaternion.
            @return Const reference to the rotation quaternion.
        */
        DLL const Quaternion& GetRotation() const;
        /** @brief Get the translation vector.
            @return Const reference to the translation vector.
        */
        DLL const Vector3& GetTranslation() const;

        /*! \brief Set transform scale vector. */
        DLL void SetScale ( const Vector3& aScale );
        /** @brief Set the rotation quaternion.
            @param aRotation The new rotation quaternion.
        */
        DLL void SetRotation ( const Quaternion& aRotation );
        /** @brief Set the translation vector.
            @param aTranslation The new translation vector.
        */
        DLL void SetTranslation ( const Vector3& aTranslation );

        /*! @brief Adds the provided vector to the position vector.
        \param x [in] X move vector value.
        \param y [in] Y move vector value.
        \param z [in] Z move vector value.
        */
        DLL void Move ( float x, float y, float z );

        /*! @brief Moves the transfrom relative to its own axes.
        \param x [in] X move vector value.
        \param y [in] Y move vector value.
        \param z [in] Z move vector value.
        */
        DLL void MoveInObjectSpace ( float x, float y, float z );

        /*!
        \brief Rotate in Object Space.
        \param angle [in] Angle in degrees of rotation.
        \param x [in] X element of axis of rotation.
        \param y [in] Y element of axis of rotation.
        \param z [in] Z element of axis of rotation.
        */
        DLL void RotateObjectSpace ( float angle, float x, float y, float z );
        /*!
        \brief Rotate in Inertial Space.
        \param angle [in] Angle in degrees of rotation.
        \param x [in] X element of axis of rotation.
        \param y [in] Y element of axis of rotation.
        \param z [in] Z element of axis of rotation.
        */
        DLL void RotateInertialSpace ( float angle, float x, float y, float z );

        // This should be a message
        //void ApplyLinearVelocityInObjectSpace ( Velocity& velocity, float delta );

        /// Clear rotation.
        DLL void ResetRotation();

        /*!
        \brief Constructs a transformation matrix from the SRT variables.
        \return Transformation matrix.
        */
        DLL const Matrix4x4 GetMatrix() const;

        /*!
        \brief Constructs a transformation matrix from only the SR variables.
        \return 3x3 transformation matrix.
        */
        DLL const Matrix3x3 GetScaleRotationMatrix() const;

        /*!
        \brief Constructs an inverted transformation matrix from the SRT variables.
        \return Inverted transformation matrix.
        \note This matrix is useful as a view matrix.
        */
        DLL const Matrix4x4 GetInvertedMatrix() const;
        /** @brief Invert this transform in place.
            @return Reference to this transform after inversion.
        */
        DLL Transform& Invert();
        /** @brief Get the inverted transform.
            @return A new Transform that is the inverse of this transform.
        */
        DLL const Transform GetInverted() const;

        /*! \name Operators */
        //@{
        /// @brief Combine this transform with another transform.
        DLL Transform& operator*= ( const Transform& lhs );
        //@}

    protected:
        /// Scale rotation and translation
        Vector3 mScale{1, 1, 1};
        Quaternion mRotation{1, 0, 0, 0}; ///< Rotation quaternion.
        Vector3 mTranslation{0, 0, 0}; ///< Translation vector.
    };
    /** @brief Combine two transforms.
        @param lhs Left-hand side transform.
        @param rhs Right-hand side transform.
        @return The combined transform.
    */
    DLL const Transform operator* ( const Transform& lhs, const Transform& rhs );
    /** @brief Transform an axis-aligned bounding box.
        @param lhs The transform to apply.
        @param rhs The AABB to transform.
        @return The transformed AABB.
    */
    DLL const AABB operator* ( const Transform& lhs, const AABB& rhs );
    /** @brief Compare two transforms for equality.
        @param lhs Left-hand side transform.
        @param rhs Right-hand side transform.
        @return True if the transforms are equal.
    */
    DLL const bool operator== ( const Transform& lhs, const Transform& rhs );
    /** Interpolate transforms using spline and mlerp methods.*/
    DLL const Transform Interpolate ( const Transform& aTransform0, const Transform& aTransform1, const Transform& aTransform2, const Transform& aTransform3, double aInterpolation );
}
#endif

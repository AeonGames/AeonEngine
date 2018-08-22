/*
Copyright (C) 2013-2018 Rodrigo Jose Hernandez Cordoba

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
    \copy 2013-2016
*/

#include "aeongames/Platform.h"
#include "aeongames/Vector3.h"
#include "aeongames/Quaternion.h"

namespace AeonGames
{
    class AABB;
    /*! \brief Component class for any object that requires space transformations.
        \ingroup placeables
    */
    class Transform
    {
    public:
        DLL Transform();
        DLL Transform ( const Vector3& aScale, const Quaternion& aRotation, const Vector3& aTranslation );
        DLL Transform ( const Vector3& aScale, const Vector3& aRotation, const Vector3& aTranslation );
        DLL Transform ( const float* aData );
        DLL void Get ( float* aData ) const;

        /// destructor.
        DLL ~Transform();
        DLL const Vector3& GetScale() const;
        DLL const Quaternion& GetRotation() const;
        DLL const Vector3& GetTranslation() const;

        /*! \brief Set transform scale vector. */
        DLL void SetScale ( const Vector3& aScale );
        DLL void SetRotation ( const Quaternion& aRotation );
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
        \param M [out] Transformation matrix.
        \return Pointer to transformation matrix, same as M.
        */
        DLL const Matrix4x4 GetMatrix() const;

        /*!
        \brief Constructs an inverted transformation matrix from the SRT variables.
        \param M [out] Transformation matrix.
        \return Pointer to transformation matrix, same as M.
        \note This matrix is useful as a view matrix.
        */
        DLL const Matrix4x4 GetInvertedMatrix() const;
        DLL Transform& Invert();
        DLL const Transform GetInverted() const;

        /*! \name Operators */
        //@{
        DLL Transform& operator*= ( const Transform& lhs );
        //@}

    protected:
        /// Scale rotation and translation
        //float srt[10];
        Vector3 mScale{1, 1, 1};
        Quaternion mRotation{1, 0, 0, 0};
        Vector3 mTranslation{0, 0, 0};
    };
    DLL const Transform operator* ( const Transform& lhs, const Transform& rhs );
    DLL const AABB operator* ( const Transform& lhs, const AABB& rhs );
    DLL const bool operator== ( const Transform& lhs, const Transform& rhs );
    /** Interpolate transforms using spline and mlerp methods.*/
    DLL const Transform Interpolate ( const Transform& aTransform0, const Transform& aTransform1, const Transform& aTransform2, const Transform& aTransform3, double aInterpolation );
}
#endif

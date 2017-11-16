/*
Copyright (C) 2013-2017 Rodrigo Jose Hernandez Cordoba

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
        /*! @brief Constructor
            @param aTx X Value for the translation vector.
            @param aTy Y Value for the translation vector.
            @param aTz Z Value for the translation vector.
            @param aRw W Value for the rotation quaternion.
            @param aRx X Value for the rotation quaternion.
            @param aRy Y Value for the rotation quaternion.
            @param aRz Z Value for the rotation quaternion.
            @param aSx X Value for the scale vector.
            @param aSy Y Value for the scale vector.
            @param aSz Z Value for the scale vector.
            @note The parameters this constructor takes here are in TSR order, but are stored internaly as SRT,
            it has been done this way to allow for rotation and scale to be optionally set.
        */
        DLL Transform (
            float aTx, float aTy, float aTz,
            float aRw = 1.0f, float aRx = 0.0f, float aRy = 0.0f, float aRz = 0.0f,
            float aSx = 1.0f, float aSy = 1.0f, float aSz = 1.0f );
        /*! @brief Constructor
        @param aSRT a float pointer or array containing scale rotation and translation information in that order
        consisting of 3 values XYZ for scale, 4 values WXYZ quaternion for rotation and 3 values XYZ for translation.
        @note This is the same format as the values are internally stored and what Transform::GetTransform returns.
        */
        DLL explicit Transform ( const float* const aSRT );
        DLL Transform ( const Vector3& aScale, const Quaternion& aRotation, const Vector3& aTranslation );

        /// destructor.
        DLL ~Transform();
        DLL const Vector3& GetScale() const;
        DLL const Quaternion& GetRotation() const;
        DLL const Vector3& GetTranslation() const;

        /*! \brief Set object scale.
           \param x [in] X scale value.
           \param y [in] Y scale value.
           \param z [in] Z scale value.
        */
        DLL void SetScale ( float x, float y, float z );
        /*! \brief Set object scale.
        \param v [in] V scale vector.*/
        DLL void SetScale ( float* v );

        /*! \brief Set object X scale value.
        \param v [in] scale value.*/
        DLL void SetScaleX ( float v );
        /*! \brief Set object Y scale value.
        \param v [in] scale value.*/
        DLL void SetScaleY ( float v );
        /*! \brief Set object Z scale value.
        \param v [in] scale value.*/
        DLL void SetScaleZ ( float v );

        /*! \brief Set object rotation.
        \param x [in] X rotation value.
        \param y [in] Y rotation value.
        \param z [in] Z rotation value.
        */
        DLL void SetRotation ( float w, float x, float y, float z );
        /*! \brief Set object rotation.
        \param v [in] V rotation vector.*/
        DLL void SetRotation ( float* v );

        /*! \brief Set object W rotation value.
        \param v [in] rotation value.*/
        DLL void SetRotationW ( float v );
        /*! \brief Set object X rotation value.
        \param v [in] rotation value.*/
        DLL void SetRotationX ( float v );
        /*! \brief Set object Y rotation value.
        \param v [in] rotation value.*/
        DLL void SetRotationY ( float v );
        /*! \brief Set object Z rotation value.
        \param v [in] rotation value.*/
        DLL void SetRotationZ ( float v );

        /*! \brief Set object position.
        \param x [in] X position value.
        \param y [in] Y position value.
        \param z [in] Z position value.
        */
        DLL void SetTranslation ( float x, float y, float z );

        /*! \brief Set object position.
        \param v [in] V position vector.*/
        DLL void SetTranslation ( float* v );
        /*! \brief Set object X position value.
        \param v [in] position value.*/
        DLL void SetTranslationX ( float v );
        /*! \brief Set object Y position value.
        \param v [in] position value.*/
        DLL void SetTranslationY ( float v );
        /*! \brief Set object Z position value.
        \param v [in] position value.*/
        DLL void SetTranslationZ ( float v );


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

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

namespace AeonGames
{
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
        /// destructor.
        DLL ~Transform();
        DLL const float* const GetScale() const;
        DLL float GetScaleX() const;
        DLL float GetScaleY() const;
        DLL float GetScaleZ() const;
        DLL const float* const GetRotation() const;
        DLL float GetRotationW() const;
        DLL float GetRotationX() const;
        DLL float GetRotationY() const;
        DLL float GetRotationZ() const;
        DLL const float* const GetTranslation() const;
        DLL float GetTranslationX() const;
        DLL float GetTranslationY() const;
        DLL float GetTranslationZ() const;
        ///@todo rename to GetSRT
        DLL const float* const GetTransform() const;
        /*! \brief Retrieve object position.
            \param x [out] Reference to variable to receive X position value.
            \param y [out] Reference to variable to receive Y position value.
            \param z [out] Reference to variable to receive Z position value.
        */
        DLL void GetTranslation ( float& x, float& y, float& z ) const;
        /*! \brief Retrieve object position.
           \param v [out] Pointer to vector to receive values.
        */
        DLL void GetTranslation ( float* v ) const;

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
        DLL float* GetMatrix ( float* M ) const;

        /*!
        \brief Constructs an inverted transformation matrix from the SRT variables.
        \param M [out] Transformation matrix.
        \return Pointer to transformation matrix, same as M.
        \note This matrix is useful as a view matrix.
        */
        DLL float* GetInvertedMatrix ( float* M ) const;
        DLL Transform& Invert();
        DLL const Transform GetInverted() const;

        /*! \name Operators */
        //@{
        DLL Transform& operator*= ( const Transform& lhs );
        //@}

    protected:
        /// Scale rotation and translation vectors
        float srt[10];
    };
    DLL const Transform operator* ( const Transform& lhs, const Transform& rhs );
    DLL const bool operator== ( const Transform& lhs, const Transform& rhs );
    /** Interpolate transforms using spline and mlerp methods.*/
    DLL const Transform Interpolate ( const Transform& aTransform0, const Transform& aTransform1, const Transform& aTransform2, const Transform& aTransform3, float aInterpolation );
}
#endif
